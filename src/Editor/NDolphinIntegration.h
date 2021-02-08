#ifndef NDOLPHININTEGRATION_H
#define NDOLPHININTEGRATION_H

#include <Common/Common.h>
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
DECLARE_FLAGS_ENUMCLASS(EQuickplayFeature, FQuickplayFeatures)

/** Full parameter set for quickplay that gets passed to the game. */
struct SQuickplayParameters
{
    /** Magic/Version */
    static const uint32 kParmsMagic = 0x00BADB01;
    static const uint32 kParmsVersion = 2;

    /** Flags indicating which features are enabled. */
    FQuickplayFeatures      Features;
    /** Asset ID of the world/area to load on boot (if JumpToArea is set). */
    uint32                  BootWorldAssetID;
    uint32                  BootAreaAssetID;
    /** Explicit align to 64 bits */
    uint32                  __PADDING;
    /** Flags indicating which layers to enable on boot (if JumpToArea is set). */
    uint64                  BootAreaLayerFlags;
    /** Location to spawn the player at when the game initially starts up. */
    CTransform4f            SpawnTransform;

    /** Serialize to disk */
    void Write(const TString& kPath) const
    {
        CFileOutStream Stream(kPath, EEndian::BigEndian);
        ASSERT(Stream.IsValid());

        // Magic/Version
        Stream.WriteULong(kParmsMagic);
        Stream.WriteULong(kParmsVersion);

        // Parameters
        Stream.WriteULong(Features.ToInt32());
        Stream.WriteULong(BootWorldAssetID);
        Stream.WriteULong(BootAreaAssetID);
        Stream.WriteULong(0);
        Stream.WriteULongLong(BootAreaLayerFlags);
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
    void QuickplayFinished(int ReturnCode);
};

/** Attempt to launch quickplay based on the current editor state. */
EQuickplayLaunchResult LaunchQuickplay(QWidget* pParentWidget,
                                       CGameProject* pProject,
                                       const SQuickplayParameters& kParms);

/** Return whether quickplay is supported for the given project */
bool IsQuickplaySupported(CGameProject* pProject);

/** Kill the current quickplay process, if it exists. */
void KillQuickplay();

/** Clean up any quickplay related file data from the project disc files. */
void CleanupQuickplayFiles(CGameProject* pProject);

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
