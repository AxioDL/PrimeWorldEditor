#ifndef NDOLPHININTEGRATION_H
#define NDOLPHININTEGRATION_H

#include <Common/Flags.h>
#include <Common/FileIO/CFileOutStream.h>
#include <Common/FileIO/IOutputStream.h>
#include <Common/Math/CTransform4f.h>
#include <Core/GameProject/CGameProject.h>

#include <QProcess>
#include <QString>

// IMPORTANT NOTE: Most values, enums, and structs declared in this file
// are mirrored in PWEQuickplayPatch and are used by the game.
// If you modify one, make sure you modify the other.

/** Return value for LaunchQuickplay */
enum class EQuickplayLaunchResult
{
    Success = 0,
    AlreadyRunning = -1,
    DolphinNotSet = -2,
    UnsupportedForProject = -3,
    Failure = -10
};

/** Flags allowing for quickplay features to be toggled on/off */
enum class EQuickplayFeature
{
    /** On boot, automatically load the area specified by WorldID and AreaID */
    JumpToArea              = 0x00000001,
    /** Spawn the player in the location specified by SpawnTransform */
    SetSpawnPosition        = 0x00000002,
    /** Give the player all items on spawn */
    GiveAllItems            = 0x00000004,

    /** Flags enabled by default */
    DefaultFeatures         = JumpToArea | SetSpawnPosition
};
AXIO_DECLARE_FLAGS_ENUMCLASS(EQuickplayFeature, FQuickplayFeatures)

/** Full parameter set for quickplay that gets passed to the game. */
struct SQuickplayParameters
{
    /** Magic/Version */
    static constexpr uint32_t kParmsMagic = 0x00BADB01;
    static constexpr uint32_t kParmsVersion = 2;

    /** Flags indicating which features are enabled. */
    FQuickplayFeatures      Features;
    /** Asset ID of the world/area to load on boot (if JumpToArea is set). */
    uint32_t                BootWorldAssetID;
    uint32_t                BootAreaAssetID;
    /** Explicit align to 64 bits */
    uint32_t                __PADDING;
    /** Flags indicating which layers to enable on boot (if JumpToArea is set). */
    uint64_t                BootAreaLayerFlags;
    /** Location to spawn the player at when the game initially starts up. */
    CTransform4f            SpawnTransform;

    /** Serialize to disk */
    void Write(const TString& kPath) const
    {
        CFileOutStream Stream(kPath, std::endian::big);
        ASSERT(Stream.IsValid());

        // Magic/Version
        Stream.WriteU32(kParmsMagic);
        Stream.WriteU32(kParmsVersion);

        // Parameters
        Stream.WriteU32(Features.Value());
        Stream.WriteU32(BootWorldAssetID);
        Stream.WriteU32(BootAreaAssetID);
        Stream.WriteU32(0);
        Stream.WriteU64(BootAreaLayerFlags);
        SpawnTransform.Write(Stream);

        Stream.Close();
    }
};

namespace NDolphinIntegration
{

/** Minimal relay class for internal use that detects when the active quickplay session is closed */
class CQuickplayRelay : public QObject
{
    Q_OBJECT

public:
    CQuickplayRelay() = default;

public slots:
    void QuickplayStarted();
    void QuickplayFinished(int ReturnCode, QProcess::ExitStatus exitStatus);
};

/** Attempt to launch quickplay based on the current editor state. */
EQuickplayLaunchResult LaunchQuickplay(QWidget* pParentWidget,
                                       CGameProject* pProject,
                                       const SQuickplayParameters& kParms);

/** Return whether quickplay is supported for the given project */
bool IsQuickplaySupported(const CGameProject* pProject);

/** Kill the current quickplay process, if it exists. */
void KillQuickplay();

/** Clean up any quickplay related file data from the project disc files. */
void CleanupQuickplayFiles(const CGameProject* pProject);

/** Set the user path to Dolphin. Returns true if succeeded. */
bool SetDolphinPath(QWidget* pParentWidget,
                    const QString& kDolphinPath,
                    bool bSilent = false);

/** Retrieves the user path to Dolphin. */
QString GetDolphinPath();

/** Prompt user to select dolphin binary. */
QString AskForDolphinPath(QWidget* pParentWidget);

/** Saves/retrieves the given quickplay settings to/from QSettings. */
void SaveQuickplayParameters(const SQuickplayParameters& kParms);
void LoadQuickplayParameters(SQuickplayParameters& Parms);

}

#endif // CQUICKPLAYCONTROLLER_H
