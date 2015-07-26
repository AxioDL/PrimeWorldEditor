#ifndef CWORLD_H
#define CWORLD_H

#include "CResource.h"
#include "CGameArea.h"
#include "CStringTable.h"
#include "SDependency.h"
#include "model/CModel.h"
#include <Common/CTransform4f.h>

class CWorld : public CResource
{
    friend class CWorldLoader;

    // Instances of CResource pointers are placeholders for unimplemented resource types (eg CMapWorld)
    EGame mWorldVersion;
    CStringTable *mpWorldName;
    CStringTable *mpDarkWorldName;
    CResource *mpSaveWorld;
    CModel *mpDefaultSkybox;
    CResource *mpMapWorld;
    CToken mResTokens[5];

    u32 mUnknown1;
    u32 mUnknownAreas;

    u32 mUnknownAGSC;
    struct SAudioGrp
    {
        u32 ResID;
        u32 Unknown;
    };
    std::vector<SAudioGrp> mAudioGrps;

    struct SMemoryRelay
    {
        u32 InstanceID;
        u32 TargetID;
        u16 Message;
        u8 Unknown;
    };
    std::vector<SMemoryRelay> mMemoryRelays;

    struct SArea
    {
        std::string InternalName;
        CStringTable *pAreaName;
        CTransform4f Transform;
        CAABox AetherBox;
        u64 FileID; // Loading every single area as a CResource would be a very bad idea
        u64 AreaID;
        CToken AreaNameToken;

        std::vector<u16> AttachedAreaIDs;
        std::vector<SDependency> Dependencies;
        std::vector<std::string> RelFilenames;
        std::vector<u32> RelOffsets;
        u32 CommonDependenciesStart;

        struct SDock
        {
            struct SConnectingDock
            {
                u32 AreaIndex;
                u32 DockIndex;
            };
            std::vector<SConnectingDock> ConnectingDocks;
            CVector3f DockCoordinates[4];
        };
        std::vector<SDock> Docks;

        struct SLayer
        {
            std::string LayerName;
            bool EnabledByDefault;
            u8 LayerID[16];
            u32 LayerDependenciesStart; // Offset into Dependencies vector
        };
        std::vector<SLayer> Layers;
    };
    std::vector<SArea> mAreas;


public:
    CWorld();
    ~CWorld();
    EResType Type();

    void SetAreaLayerInfo(CGameArea *pArea, u32 AreaIndex);

    // Setters
    EGame Version();
    CStringTable* GetWorldName();
    CStringTable* GetDarkWorldName();
    CResource* GetSaveWorld();
    CModel* GetDefaultSkybox();
    CResource* GetMapWorld();

    u32 GetNumAreas();
    u64 GetAreaResourceID(u32 AreaIndex);
    u32 GetAreaAttachedCount(u32 AreaIndex);
    u32 GetAreaAttachedID(u32 AreaIndex, u32 AttachedIndex);
    std::string GetAreaInternalName(u32 AreaIndex);
    CStringTable* GetAreaName(u32 AreaIndex);
};

#endif // CWORLD_H
