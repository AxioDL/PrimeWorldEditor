#ifndef CWORLD_H
#define CWORLD_H

#include "CResource.h"
#include "CSavedStateID.h"
#include "Core/Resource/Area/CGameArea.h"
#include "Core/Resource/Model/CModel.h"
#include "Core/Resource/StringTable/CStringTable.h"
#include <Common/Math/CTransform4f.h>

class CWorld : public CResource
{
    DECLARE_RESOURCE_TYPE(World)
    friend class CWorldLoader;
    friend class CWorldCooker;

    // Instances of CResource pointers are placeholders for unimplemented resource types (eg CMapWorld)
    TString mName;
    TResPtr<CStringTable> mpWorldName;
    TResPtr<CStringTable> mpDarkWorldName;
    TResPtr<CResource>    mpSaveWorld;
    TResPtr<CModel>       mpDefaultSkybox;
    TResPtr<CResource>    mpMapWorld;
    uint32 mTempleKeyWorldIndex;

    struct STimeAttackData
    {
        bool HasTimeAttack;
        TString ActNumber;
        float BronzeTime;
        float SilverTime;
        float GoldTime;
        float ShinyGoldTime;
    } mTimeAttackData;

    struct SAudioGrp
    {
        CAssetID ResID;
        uint32 GroupID;
    };
    std::vector<SAudioGrp> mAudioGrps;

    struct SMemoryRelay
    {
        uint32 InstanceID;
        uint32 TargetID;
        uint16 Message;
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
        std::vector<uint16> AttachedAreaIDs;
        std::vector<TString> RelFilenames; // Needs to be removed & generated at cook; temporarily leaving for debugging
        std::vector<uint32> RelOffsets;

        struct SDock
        {
            struct SConnectingDock
            {
                uint32 AreaIndex;
                uint32 DockIndex;
            };
            std::vector<SConnectingDock> ConnectingDocks;
            std::vector<CVector3f> DockCoordinates;
        };
        std::vector<SDock> Docks;

        struct SLayer
        {
            TString LayerName;
            bool Active;
            CSavedStateID LayerStateID;
        };
        std::vector<SLayer> Layers;
    };
    std::vector<SArea> mAreas;

public:
    CWorld(CResourceEntry *pEntry = 0);
    ~CWorld();

    CDependencyTree* BuildDependencyTree() const;
    void SetAreaLayerInfo(CGameArea *pArea);
    TString InGameName() const;
    TString AreaInGameName(uint32 AreaIndex) const;
    uint32 AreaIndex(CAssetID AreaID) const;

    // Serialization
    virtual void Serialize(IArchive& rArc);
    friend void Serialize(IArchive& rArc, STimeAttackData& rTimeAttackData);
    friend void Serialize(IArchive& rArc, SMemoryRelay& rMemRelay);
    friend void Serialize(IArchive& rArc, SArea& rArea);
    friend void Serialize(IArchive& rArc, SArea::SDock& rDock);
    friend void Serialize(IArchive& rArc, SArea::SDock::SConnectingDock& rDock);
    friend void Serialize(IArchive& rArc, SArea::SLayer& rLayer);
    friend void Serialize(IArchive& rArc, SAudioGrp& rAudioGrp);

    // Accessors
    inline TString Name() const                 { return mName; }
    inline CStringTable* NameString() const     { return mpWorldName; }
    inline CStringTable* DarkNameString() const { return mpDarkWorldName; }
    inline CResource* SaveWorld() const         { return mpSaveWorld; }
    inline CModel* DefaultSkybox() const        { return mpDefaultSkybox; }
    inline CResource* MapWorld() const          { return mpMapWorld; }

    inline uint32 NumAreas() const                                              { return mAreas.size(); }
    inline CAssetID AreaResourceID(uint32 AreaIndex) const                      { return mAreas[AreaIndex].AreaResID; }
    inline uint32 AreaAttachedCount(uint32 AreaIndex) const                     { return mAreas[AreaIndex].AttachedAreaIDs.size(); }
    inline uint32 AreaAttachedID(uint32 AreaIndex, uint32 AttachedIndex) const  { return mAreas[AreaIndex].AttachedAreaIDs[AttachedIndex]; }
    inline TString AreaInternalName(uint32 AreaIndex) const                     { return mAreas[AreaIndex].InternalName; }
    inline CStringTable* AreaName(uint32 AreaIndex) const                       { return mAreas[AreaIndex].pAreaName; }
    inline bool DoesAreaAllowPakDuplicates(uint32 AreaIndex) const              { return mAreas[AreaIndex].AllowPakDuplicates; }

    inline void SetName(const TString& rkName)                              { mName = rkName; }
    inline void SetAreaAllowsPakDuplicates(uint32 AreaIndex, bool Allow)    { mAreas[AreaIndex].AllowPakDuplicates = Allow; }
};

#endif // CWORLD_H
