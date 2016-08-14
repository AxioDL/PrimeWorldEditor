#ifndef CWORLD_H
#define CWORLD_H

#include "CResource.h"
#include "CStringTable.h"
#include "Core/Resource/Area/CGameArea.h"
#include "Core/Resource/Model/CModel.h"
#include <Math/CTransform4f.h>

class CWorld : public CResource
{
    DECLARE_RESOURCE_TYPE(eWorld)
    friend class CWorldLoader;
    friend class CWorldCooker;

    // Instances of CResource pointers are placeholders for unimplemented resource types (eg CMapWorld)
    TResPtr<CStringTable> mpWorldName;
    TResPtr<CStringTable> mpDarkWorldName;
    TResPtr<CResource>    mpSaveWorld;
    TResPtr<CModel>       mpDefaultSkybox;
    TResPtr<CResource>    mpMapWorld;
    u32 mTempleKeyWorldIndex;

    struct SAudioGrp
    {
        CAssetID ResID;
        u32 GroupID;
    };
    std::vector<SAudioGrp> mAudioGrps;

    struct SMemoryRelay
    {
        u32 InstanceID;
        u32 TargetID;
        u16 Message;
        bool Active;
    };
    std::vector<SMemoryRelay> mMemoryRelays;

    struct SArea
    {
        TString InternalName;
        TResPtr<CStringTable> pAreaName;
        CTransform4f Transform;
        CAABox AetherBox;
        CAssetID AreaResID; // Area resource ID
        CAssetID AreaID; // Internal area ID (same length as an asset ID)
        bool AllowPakDuplicates;

        std::vector<SMemoryRelay> MemoryRelays; // Only needed for MP1
        std::vector<u16> AttachedAreaIDs;
        std::vector<TString> RelFilenames; // Needs to be removed & generated at cook; temporarily leaving for debugging
        std::vector<u32> RelOffsets;

        struct SDock
        {
            struct SConnectingDock
            {
                u32 AreaIndex;
                u32 DockIndex;
            };
            std::vector<SConnectingDock> ConnectingDocks;
            std::vector<CVector3f> DockCoordinates;
        };
        std::vector<SDock> Docks;

        struct SLayer
        {
            TString LayerName;
            bool Active;
            u8 LayerID[16];
        };
        std::vector<SLayer> Layers;
    };
    std::vector<SArea> mAreas;

public:
    CWorld(CResourceEntry *pEntry = 0);
    ~CWorld();

    CDependencyTree* BuildDependencyTree() const;
    void SetAreaLayerInfo(CGameArea *pArea);

    // Serialization
    virtual void Serialize(IArchive& rArc);
    friend void Serialize(IArchive& rArc, SMemoryRelay& rMemRelay);
    friend void Serialize(IArchive& rArc, SArea& rArea);
    friend void Serialize(IArchive& rArc, SArea::SDock& rDock);
    friend void Serialize(IArchive& rArc, SArea::SDock::SConnectingDock& rDock);
    friend void Serialize(IArchive& rArc, SArea::SLayer& rLayer);
    friend void Serialize(IArchive& rArc, SAudioGrp& rAudioGrp);

    // Accessors
    inline CStringTable* WorldName() const      { return mpWorldName; }
    inline CStringTable* DarkWorldName() const  { return mpDarkWorldName; }
    inline CResource* SaveWorld() const         { return mpSaveWorld; }
    inline CModel* DefaultSkybox() const        { return mpDefaultSkybox; }
    inline CResource* MapWorld() const          { return mpMapWorld; }

    inline u32 NumAreas() const                                         { return mAreas.size(); }
    inline CAssetID AreaResourceID(u32 AreaIndex) const                 { return mAreas[AreaIndex].AreaResID; }
    inline u32 AreaAttachedCount(u32 AreaIndex) const                   { return mAreas[AreaIndex].AttachedAreaIDs.size(); }
    inline u32 AreaAttachedID(u32 AreaIndex, u32 AttachedIndex) const   { return mAreas[AreaIndex].AttachedAreaIDs[AttachedIndex]; }
    inline TString AreaInternalName(u32 AreaIndex) const                { return mAreas[AreaIndex].InternalName; }
    inline CStringTable* AreaName(u32 AreaIndex) const                  { return mAreas[AreaIndex].pAreaName; }
    inline bool DoesAreaAllowPakDuplicates(u32 AreaIndex) const         { return mAreas[AreaIndex].AllowPakDuplicates; }

    inline void SetAreaAllowsPakDuplicates(u32 AreaIndex, bool Allow)   { mAreas[AreaIndex].AllowPakDuplicates = Allow; }
};

#endif // CWORLD_H
