#include "NDolphinIntegration.h"
#include "Editor/UICommon.h"

#include <QFileInfo>
#include <QRegExp>
#include <QObject>
#include <QSettings>

namespace NDolphinIntegration
{

/** Constants */
const char* const gkDolphinPathSetting  = "Quickplay/DolphinPath";
const char* const gkFeaturesSetting     = "Quickplay/Features";

const char* const gkRelFileName         = "EditorQuickplay.rel";
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
    connect(gpDolphinProcess, SIGNAL(finished(int)), this, SLOT(QuickplayFinished(int)));
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

/** Attempt to launch quickplay based on the current editor state. */
EQuickplayLaunchResult LaunchQuickplay(QWidget* pParentWidget,
                                       CGameProject* pProject,
                                       const SQuickplayParameters& kParms)
{
    debugf("Launching quickplay...");

    // Check if quickplay is supported for this project
    TString QuickplayDir = "../resources/quickplay" / ::GetGameShortName(pProject->Game());
    TString BuildString = "v" + TString::FromFloat(pProject->BuildVersion());
    TString RelFile = QuickplayDir / BuildString + ".rel";
    TString PatchFile = QuickplayDir / BuildString + ".bin";

    if (!FileUtil::Exists(RelFile) || !FileUtil::Exists(PatchFile))
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
            Path = UICommon::OpenFileDialog(pParentWidget, "Open Dolphin", "Dolphin.exe");
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

    bool bLoadedDol = FileUtil::LoadFileToBuffer(DiscSys / "main.dol", DolData);
    bool bLoadedPatch = FileUtil::LoadFileToBuffer(PatchFile, PatchData);

    if (!bLoadedDol || !bLoadedPatch)
    {
        warnf("Quickplay launch failed! Failed to load %s into memory.",
              bLoadedDol ? "patch data" : "game DOL");

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

    // Append the patch data to the end of the dol
    uint32 AlignedDolSize = ALIGN(DolData.size(), 32);
    uint32 AlignedPatchSize = ALIGN(PatchData.size(), 32);
    uint32 PatchOffset = AlignedDolSize;
    uint32 PatchSize = AlignedPatchSize;
    DolData.resize(PatchOffset + PatchSize);
    memcpy(&DolData[PatchOffset], &PatchData[0], PatchData.size());

    // These constants are hardcoded for the moment.
    // We write the patch to text section 6, which must be at address 0x80002600.
    // We hook over the call to LCEnable, which is at offset 0x1D64.
    CMemoryOutStream Mem(DolData.data(), DolData.size(), EEndian::BigEndian);
    Mem.GoTo(0x18);
    Mem.WriteLong(PatchOffset);
    Mem.GoTo(0x60);
    Mem.WriteLong(0x80002600);
    Mem.GoTo(0xA8);
    Mem.WriteLong(PatchSize);
    Mem.GoTo(0x1D64);
    Mem.WriteLong(0x4BFFD80D); // this patches in a call to the quickplay bootstrap during game boot process

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
    TString QuickplayDir = "../resources/quickplay" / ::GetGameShortName(pProject->Game());
    TString BuildString = "v" + TString::FromFloat(pProject->BuildVersion());
    TString RelFile = QuickplayDir / BuildString + ".rel";
    TString PatchFile = QuickplayDir / BuildString + ".bin";
    return FileUtil::Exists(RelFile) && FileUtil::Exists(PatchFile);
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

    if (!DolphinFile.exists() || DolphinFile.suffix() != "exe")
    {
        if (!bSilent)
        {
            UICommon::ErrorMsg(pParentWidget, "The selected file is not a Dolphin exe!");
        }
        return false;
    }

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
    }

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
