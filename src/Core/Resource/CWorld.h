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
    uint32 mTempleKeyWorldIndex = 0;

    struct STimeAttackData
    {
        bool HasTimeAttack;
        TString ActNumber;
        float BronzeTime;
        float SilverTime;
        float GoldTime;
        float ShinyGoldTime;
    };
    STimeAttackData mTimeAttackData{};

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
    explicit CWorld(CResourceEntry *pEntry = nullptr);
    ~CWorld() override;

    std::unique_ptr<CDependencyTree> BuildDependencyTree() const override;
    void SetAreaLayerInfo(CGameArea *pArea);
    TString InGameName() const;
    TString AreaInGameName(uint32 AreaIndex) const;
    uint32 AreaIndex(CAssetID AreaID) const;

    // Serialization
    void Serialize(IArchive& rArc) override;
    friend void Serialize(IArchive& rArc, STimeAttackData& rTimeAttackData);
    friend void Serialize(IArchive& rArc, SMemoryRelay& rMemRelay);
    friend void Serialize(IArchive& rArc, SArea& rArea);
    friend void Serialize(IArchive& rArc, SArea::SDock& rDock);
    friend void Serialize(IArchive& rArc, SArea::SDock::SConnectingDock& rDock);
    friend void Serialize(IArchive& rArc, SArea::SLayer& rLayer);
    friend void Serialize(IArchive& rArc, SAudioGrp& rAudioGrp);

    // Accessors
    TString Name() const                 { return mName; }
    CStringTable* NameString() const     { return mpWorldName; }
    CStringTable* DarkNameString() const { return mpDarkWorldName; }
    CResource* SaveWorld() const         { return mpSaveWorld; }
    CModel* DefaultSkybox() const        { return mpDefaultSkybox; }
    CResource* MapWorld() const          { return mpMapWorld; }

    size_t NumAreas() const                                              { return mAreas.size(); }
    CAssetID AreaResourceID(size_t AreaIndex) const                      { return mAreas[AreaIndex].AreaResID; }
    uint32 AreaAttachedCount(size_t AreaIndex) const                     { return mAreas[AreaIndex].AttachedAreaIDs.size(); }
    uint32 AreaAttachedID(size_t AreaIndex, size_t AttachedIndex) const  { return mAreas[AreaIndex].AttachedAreaIDs[AttachedIndex]; }
    TString AreaInternalName(size_t AreaIndex) const                     { return mAreas[AreaIndex].InternalName; }
    CStringTable* AreaName(size_t AreaIndex) const                       { return mAreas[AreaIndex].pAreaName; }
    bool DoesAreaAllowPakDuplicates(size_t AreaIndex) const              { return mAreas[AreaIndex].AllowPakDuplicates; }

    void SetName(TString rkName)                                     { mName = std::move(rkName); }
    void SetAreaAllowsPakDuplicates(size_t AreaIndex, bool Allow)    { mAreas[AreaIndex].AllowPakDuplicates = Allow; }
};

#endif // CWORLD_H
