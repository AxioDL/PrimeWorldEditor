#include "NDolphinIntegration.h"
#include "Editor/MacOSExtras.h"
#include "Editor/UICommon.h"
#include "Editor/SDolHeader.h"

#include <QFileInfo>
#include <QRegExp>
#include <QObject>
#include <QSettings>

#undef CopyFile
#undef MoveFile
#undef DeleteFile

namespace NDolphinIntegration
{

/** Constants */
const char* const gkDolphinPathSetting  = "Quickplay/DolphinPath";
const char* const gkFeaturesSetting     = "Quickplay/Features";

const char* const gkRelFileName         = "patches.rel";
const char* const gkParameterFile       = "dbgconfig";
const char* const gkDolPath             = "sys/main.dol";
const char* const gkDolBackupPath       = "sys/main.original.dol";

/** The user's path to the Dolphin exe */
QString gDolphinPath;

/** The current Dolphin quickplay process */
QProcess* gpDolphinProcess = nullptr;

/** The project that the current active quickplay session is running for */
CGameProject* gpQuickplayProject = nullptr;

/** Quickplay relay implementation to detect when the active quickplay session closes */
void CQuickplayRelay::QuickplayStarted()
{
    debugf("Quickplay session started.");
    connect(gpDolphinProcess, qOverload<int>(&QProcess::finished), this, &CQuickplayRelay::QuickplayFinished);
}

void CQuickplayRelay::QuickplayFinished(int ReturnCode)
{
    debugf("Quickplay session finished.");
    disconnect(gpDolphinProcess, 0, this, 0);
    CleanupQuickplayFiles(gpQuickplayProject);
    gpDolphinProcess->waitForFinished();
    gpDolphinProcess->deleteLater();
    gpDolphinProcess = nullptr;
    gpQuickplayProject = nullptr;
}

CQuickplayRelay gQuickplayRelay;

uint32 AssembleBranchInstruction(uint32 instructionAddress, uint32 branchTarget)
{
    int32 jumpOffset = ((int32)branchTarget - (int32)instructionAddress) / 4;
    if (jumpOffset < 0)
    {
        jumpOffset += 1 << 24;
    }
    return (18 << 26) + (jumpOffset << 2);
}

std::map<TString, uint32> LoadSymbols(const TString& mapContents) {
    std::map<TString, uint32> result;

    for (auto& line : mapContents.Split("\n"))
    {
        auto separator = line.IndexOf(' ');
        if (separator > 0)
        {
            auto address = line.SubString(0, separator);
            auto name = line.SubString(separator + 1, line.Length() - separator - 1).Trimmed();

            result.emplace(name, static_cast<uint32>(address.ToInt32(16)));
        }
    }

    return result;
}

/** Attempt to launch quickplay based on the current editor state. */
EQuickplayLaunchResult LaunchQuickplay(QWidget* pParentWidget,
                                       CGameProject* pProject,
                                       const SQuickplayParameters& kParms)
{
    debugf("Launching quickplay...");

    // Check if we have the files needed for this project's target game and version.
    // The required files are split into two parts:
    // * Quickplay Module: A .rel compiled from https://github.com/AxioDL/PWEQuickplayPatch/
    // * Dol patch for loading RELs:
    //   - A .bin compiled from https://github.com/aprilwade/randomprime/tree/master/compile_to_ppc/rel_loader
    //   - A .map, with the symbol for the .bin and the function in the dol to patch.
    TString QuickplayDir = gDataDir + "resources/quickplay" / ::GetGameShortName(pProject->Game());
    TString BuildString = "v" + TString::FromFloat(pProject->BuildVersion());
    TString RelFile = QuickplayDir / BuildString + ".rel";
    TString PatchFile = QuickplayDir / BuildString + ".bin";
    TString PatchMapFile = QuickplayDir / BuildString + ".map";

    if (!FileUtil::Exists(RelFile) || !FileUtil::Exists(PatchFile) || !FileUtil::Exists(PatchMapFile))
    {
        warnf("Quickplay launch failed! Quickplay is not supported for this project.");
        return EQuickplayLaunchResult::UnsupportedForProject;
    }

    // Check if quickplay is already running
    if (gpDolphinProcess && gpDolphinProcess->state() != QProcess::NotRunning)
    {
        if (UICommon::YesNoQuestion(pParentWidget, "Quickplay",
            "Quickplay is already running. Close the existing session and relaunch?"))
        {
            KillQuickplay();
        }
        else
        {
            warnf("Quickplay launch failed! User is already running quickplay.");
            return EQuickplayLaunchResult::AlreadyRunning;
        }
    }

    // Check if we have a path to Dolphin
    if (gDolphinPath.isEmpty())
    {
        // If the user configured Dolphin on a previous run, it should be stashed in settings
        QString Path = GetDolphinPath();

        if (Path.isEmpty())
        {
            // Allow the user to select the Dolphin exe.
            Path = AskForDolphinPath(pParentWidget);
        }

        bool bGotDolphin = (!Path.isEmpty() && SetDolphinPath(pParentWidget, Path, true));

        if (!bGotDolphin)
        {
            // We don't have Dolphin
            warnf("Quickplay launch failed! Dolphin is not configured.");
            return EQuickplayLaunchResult::DolphinNotSet;
        }
    }

    // Check if the user has any dirty packages
    if (gpEdApp->HasAnyDirtyPackages())
    {
        if (UICommon::YesNoQuestion(pParentWidget, "Uncooked Changes",
            "You have uncooked changes. Cook before starting quickplay?"))
        {
            gpEdApp->CookAllDirtyPackages();
        }
    }

    // All good. Perform initialization tasks. Start by creating the patched dol.
    TString DiscSys = pProject->DiscDir(false) / "sys";
    std::vector<uint8> DolData;
    std::vector<uint8> PatchData;
    TString MapData;

    bool bLoadedDol = FileUtil::LoadFileToBuffer(DiscSys / "main.dol", DolData);
    bool bLoadedPatch = FileUtil::LoadFileToBuffer(PatchFile, PatchData);
    bool bLoadedMap = FileUtil::LoadFileToString(PatchMapFile, MapData);

    if (!bLoadedDol || !bLoadedPatch || !bLoadedMap)
    {
        const char* failedPart = !bLoadedDol ? "game DOL" : (!bLoadedPatch ? "patch data" : "patch symbols");
        warnf("Quickplay launch failed! Failed to load %s into memory.", failedPart);

        return EQuickplayLaunchResult::Failure;
    }

    // Back up the original dol.
    // Note that Dolphin requires the dol to be located at sys/main.dol, which
    // is why this is needed as a workaround.
    TString DolPath = DiscSys / "main.dol";
    TString DolBackupPath = DiscSys / "main.original.dol";

    // Note we want to make sure there is no existing backup before copying.
    // There may be a case where some quickplay files were left over from an old
    // session that didn't terminate correctly. In this case, main.dol will be
    // the quickplay dol, and this operation would overwrite the original dol.
    if (!FileUtil::Exists(DolBackupPath))
    {
        FileUtil::CopyFile(DolPath, DolBackupPath);
    }

    auto symbols = LoadSymbols(MapData);
    auto inStream = CMemoryInStream(DolData.data(), DolData.size(), EEndian::BigEndian);
    SDolHeader header(inStream);

    // Append the patch data to the end of the dol
    uint32 AlignedDolSize = VAL_ALIGN(DolData.size(), 32);
    uint32 AlignedPatchSize = VAL_ALIGN(PatchData.size(), 32);
    DolData.resize(AlignedDolSize + AlignedPatchSize);
    memcpy(&DolData[AlignedDolSize], &PatchData[0], PatchData.size());

    if (!header.AddTextSection(0x80002000, AlignedDolSize, AlignedPatchSize))
    {
        warnf("Quickplay launch failed! Failed to patch DOL. Is it already patched?");
        return EQuickplayLaunchResult::Failure;
    }

    uint32 callToHook = symbols["PPCSetFpIEEEMode"] + 4;
    uint32 branchTarget = symbols["rel_loader_hook"];

    CMemoryOutStream Mem(DolData.data(), DolData.size(), EEndian::BigEndian);
    header.Write(Mem);
    Mem.GoTo(header.OffsetForAddress(callToHook));
    Mem.WriteLong(AssembleBranchInstruction(callToHook, branchTarget));

    if (!FileUtil::SaveBufferToFile(DolPath, DolData))
    {
        warnf("Quickplay launch failed! Failed to write patched DOL.");
        return EQuickplayLaunchResult::Failure;
    }

    // Write other disc files that are needed by quickplay
    TString FSTRoot = pProject->DiscFilesystemRoot(false);
    TString FSTRelPath = FSTRoot / gkRelFileName;
    TString FSTParmPath = FSTRoot / gkParameterFile;
    FileUtil::DeleteFile(FSTRelPath);
    FileUtil::DeleteFile(FSTParmPath);
    FileUtil::CopyFile(RelFile, FSTRelPath);
    kParms.Write(FSTParmPath);

    // We're good to go - launch the quickplay process
    gpDolphinProcess = new QProcess(pParentWidget);
    gpDolphinProcess->start(gDolphinPath, QStringList() << TO_QSTRING(DolPath));
    gpDolphinProcess->waitForStarted();

    if (gpDolphinProcess->state() != QProcess::Running)
    {
        warnf("Quickplay launch failed! Process did not start correctly.");
        delete gpDolphinProcess;
        gpDolphinProcess = nullptr;
        return EQuickplayLaunchResult::Failure;
    }

    gpQuickplayProject = pProject;
    gQuickplayRelay.QuickplayStarted();
    return EQuickplayLaunchResult::Success;
}

/** Return whether quickplay is supported for the given project */
bool IsQuickplaySupported(CGameProject* pProject)
{
    // Quickplay is supported if there is a quickplay module & patch in the resources folder
    TString QuickplayDir = gDataDir + "resources/quickplay" / ::GetGameShortName(pProject->Game());
    TString BuildString = "v" + TString::FromFloat(pProject->BuildVersion());
    TString RelFile = QuickplayDir / BuildString + ".rel";
    TString PatchFile = QuickplayDir / BuildString + ".bin";
    TString MapFile = QuickplayDir / BuildString + ".map";
    return FileUtil::Exists(RelFile) && FileUtil::Exists(PatchFile) && FileUtil::Exists(MapFile);
}

/** Kill the current quickplay process, if it exists. */
void KillQuickplay()
{
    if (gpDolphinProcess)
    {
        debugf("Stopping active quickplay session.");
        gpDolphinProcess->close();
        // CQuickplayRelay handles remaining destruction & cleanup
    }
}

/** Clean up any quickplay related file data from the project disc files. */
void CleanupQuickplayFiles(CGameProject* pProject)
{
    if( pProject )
    {
        TString DiscSys = pProject->DiscDir(false) / "sys";
        TString DolPath = DiscSys / "main.dol";
        TString BackupDolPath = DiscSys / "main.original.dol";

        if (FileUtil::Exists(BackupDolPath))
        {
            FileUtil::DeleteFile(DolPath);
            FileUtil::MoveFile(BackupDolPath, DolPath);
        }

        TString FSTRoot = pProject->DiscFilesystemRoot(false);
        FileUtil::DeleteFile(FSTRoot / gkParameterFile);
        FileUtil::DeleteFile(FSTRoot / gkRelFileName);
    }
}

/** Set the user path to Dolphin */
bool SetDolphinPath(QWidget* pParentWidget, const QString& kDolphinPath, bool bSilent /*= false*/)
{
    // Validate if this is a Dolphin build
    //@todo Is there a way to check if this exe is Dolphin specifically?
    //@todo Validate the build version to make sure the build supports quickplay? Necessary?
    QFileInfo DolphinFile(kDolphinPath);

    if (!DolphinFile.exists() || !DolphinFile.isExecutable())
    {
        if (!bSilent)
        {
            UICommon::ErrorMsg(pParentWidget, "The selected file is not a Dolphin executable!");
        }
        return false;
    }

    // Try to obtain the version from Dolphin
    QProcess DolphinProcess;
    DolphinProcess.start(kDolphinPath, QStringList() << "--version");
    DolphinProcess.waitForFinished();
    QString VersionString = DolphinProcess.readLine().trimmed();
    
    // Make sure we have a valid string
    if (VersionString.isNull())
    {
        if (!bSilent)
        {
            UICommon::ErrorMsg(pParentWidget, "Unable to validate version string, the selected file is likely not a Dolphin executable");
        }
        return false;
    }

    // Dolphin's version string is broken into two parts, the first part is the name the second is the version
    // Dolphin unfortunately collide's with KDE Plasma's file manager which also happens to be named "Dolphin"
    // Fortunately the latter uses a lowercase name so we can differentiate.
    QStringList VersionParts = VersionString.split(' ');
    if (VersionParts.count() != 2 || VersionParts[0] != "Dolphin")
    {
        if (!bSilent)
        {
            UICommon::ErrorMsg(pParentWidget, "The selected file is not a Dolphin executable!");
        }
        return false;
    }

    debugf("Found dolphin version %s", *TO_TSTRING(VersionParts[1]));

    // Build is legit, stash it
    QSettings Settings;
    Settings.setValue(gkDolphinPathSetting, kDolphinPath);

    gDolphinPath = kDolphinPath;
    debugf("Setting Dolphin path to %s.", *TO_TSTRING(kDolphinPath));

    return true;
}

/** Retrieves the user path to Dolphin. */
QString GetDolphinPath()
{
    // Check if we have a path to Dolphin
    QString Path = gDolphinPath;

    if (Path.isEmpty())
    {
        // If the user configured Dolphin on a previous run, it should be stashed in settings
        QSettings Settings;
        Path = Settings.value(gkDolphinPathSetting).toString();

        if (!Path.isEmpty())
        {
            gDolphinPath = Path;
        }
        else
        {
#if defined(__APPLE__)
            gDolphinPath = MacOSPathToDolphinBinary();
#endif
        }
    }

    return Path;
}

QString AskForDolphinPath(QWidget* pParentWidget) {
#if defined(Q_OS_WIN)
    QString Path = UICommon::OpenFileDialog(pParentWidget, "Open Dolphin", "*.exe");
#elif defined(Q_OS_MACOS)
    QString Path = UICommon::OpenFileDialog(pParentWidget, "Open Dolphin", "*.app");
    if (Path.endsWith(".app"))
        Path += "/Contents/MacOS/Dolphin";
#else
    QString Path = UICommon::OpenFileDialog(pParentWidget, "Open Dolphin", {});
#endif
    return Path;
}

/** Saves/retrieves the given quickplay settings to/from QSettings. */
void SaveQuickplayParameters(const SQuickplayParameters& kParms)
{
    QSettings Settings;
    Settings.setValue(gkFeaturesSetting, kParms.Features.ToInt32());
}

void LoadQuickplayParameters(SQuickplayParameters& Parms)
{
    QSettings Settings;
    Parms.Features = Settings.value(gkFeaturesSetting, (uint32) EQuickplayFeature::DefaultFeatures).toInt();
}

}
