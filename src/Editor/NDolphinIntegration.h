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

namespace NDolphinIntegration
{

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
};
DECLARE_FLAGS_ENUMCLASS(EQuickplayFeature, FQuickplayFeatures)

/** Full parameter set for quickplay that gets passed to the game. */
struct SQuickplayParameters
{
    /** Magic/Version */
    static const uint32 kParmsMagic = 0x00BADB01;
    static const uint32 kParmsVersion = 1;

    /** Flags indicating which features are enabled. */
    FQuickplayFeatures      Features;
    /** Asset ID of the world/area to load on boot (if JumpToArea is set). */
    uint32                  BootWorldAssetID;
    uint32                  BootAreaAssetID;
    /** Location to spawn the player at when the game initially starts up. */
    CTransform4f            SpawnTransform;

    /** Serialize to disk */
    void Write(const TString& kPath) const
    {
        CFileOutStream Stream(kPath, EEndian::BigEndian);
        ASSERT( Stream.IsValid() );

        // Magic/Version
        Stream.WriteLong(kParmsMagic);
        Stream.WriteLong(kParmsVersion);

        // Parameters
        Stream.WriteLong( Features.ToInt32() );
        Stream.WriteLong( BootWorldAssetID );
        Stream.WriteLong( BootAreaAssetID );
        SpawnTransform.Write( Stream );

        Stream.Close();
    }
};

/** Minimal relay class that detects when the active quickplay session is closed */
class CQuickplayRelay : public QObject
{
    Q_OBJECT

public:
    CQuickplayRelay() {}
    void TrackProcess(QProcess* pProcess);

public slots:
    void OnQuickplayFinished(int ReturnCode);
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

}

#endif // CQUICKPLAYCONTROLLER_H
