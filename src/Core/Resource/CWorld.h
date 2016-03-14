#ifndef CWORLD_H
#define CWORLD_H

#include "CResource.h"
#include "CGameArea.h"
#include "CStringTable.h"
#include "SDependency.h"
#include "Core/Resource/Model/CModel.h"
#include <Math/CTransform4f.h>

class CWorld : public CResource
{
    DECLARE_RESOURCE_TYPE(eWorld)
    friend class CWorldLoader;

    // Instances of CResource pointers are placeholders for unimplemented resource types (eg CMapWorld)
    EGame mWorldVersion;
    TResPtr<CStringTable> mpWorldName;
    TResPtr<CStringTable> mpDarkWorldName;
    TResPtr<CResource>    mpSaveWorld;
    TResPtr<CModel>       mpDefaultSkybox;
    TResPtr<CResource>    mpMapWorld;

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
        TString InternalName;
        TResPtr<CStringTable> pAreaName;
        CTransform4f Transform;
        CAABox AetherBox;
        u64 FileID; // Loading every single area as a CResource would be a very bad idea
        u64 AreaID;

        std::vector<u16> AttachedAreaIDs;
        std::vector<SDependency> Dependencies;
        std::vector<TString> RelFilenames;
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
            TString LayerName;
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

    void SetAreaLayerInfo(CGameArea *pArea);

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
    TString GetAreaInternalName(u32 AreaIndex);
    CStringTable* GetAreaName(u32 AreaIndex);
};

#endif // CWORLD_H
