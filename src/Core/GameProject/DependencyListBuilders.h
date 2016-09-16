#ifndef DEPENDENCYLISTBUILDERS
#define DEPENDENCYLISTBUILDERS

#include "CDependencyTree.h"
#include "CGameProject.h"
#include "CPackage.h"
#include "CResourceEntry.h"
#include "Core/Resource/CDependencyGroup.h"
#include "Core/Resource/CWorld.h"

class CCharacterUsageMap
{
    std::map<CAssetID, std::vector<bool>> mUsageMap;
    std::set<CAssetID> mStillLookingIDs;
    u32 mLayerIndex;
    bool mIsInitialArea;
    bool mCurrentAreaAllowsDupes;

public:
    CCharacterUsageMap() : mLayerIndex(-1), mIsInitialArea(true), mCurrentAreaAllowsDupes(false) {}
    bool IsCharacterUsed(const CAssetID& rkID, u32 CharacterIndex) const;
    void FindUsagesForArea(CWorld *pWorld, CResourceEntry *pEntry);
    void FindUsagesForArea(CWorld *pWorld, u32 AreaIndex);
    void FindUsagesForLayer(CResourceEntry *pAreaEntry, u32 LayerIndex);
    void DebugPrintContents();

protected:
    void ParseDependencyNode(IDependencyNode *pNode);
};

// ************ CPackageDependencyListBuilder ************
class CPackageDependencyListBuilder
{
    CPackage *mpPackage;
    EGame mGame;
    TResPtr<CWorld> mpWorld;
    CCharacterUsageMap mCharacterUsageMap;
    std::set<CAssetID> mPackageUsedAssets;
    std::set<CAssetID> mAreaUsedAssets;
    bool mEnableDuplicates;
    bool mCurrentAreaHasDuplicates;
    bool mIsPlayerActor;

public:
    CPackageDependencyListBuilder(CPackage *pPackage)
        : mpPackage(pPackage)
        , mGame(pPackage->Project()->Game())
        , mCurrentAreaHasDuplicates(false)
        , mIsPlayerActor(false)
    {}

    void BuildDependencyList(bool AllowDuplicates, std::list<CAssetID>& rOut);
    void AddDependency(CResourceEntry *pCurEntry, const CAssetID& rkID, std::list<CAssetID>& rOut);
    void EvaluateDependencyNode(CResourceEntry *pCurEntry, IDependencyNode *pNode, std::list<CAssetID>& rOut);
};

// ************ CAreaDependencyListBuilder ************
class CAreaDependencyListBuilder
{
    CResourceEntry *mpAreaEntry;
    EGame mGame;
    CCharacterUsageMap mCharacterUsageMap;
    std::set<CAssetID> mBaseUsedAssets;
    std::set<CAssetID> mLayerUsedAssets;
    bool mIsPlayerActor;

public:
    CAreaDependencyListBuilder(CResourceEntry *pAreaEntry)
        : mpAreaEntry(pAreaEntry)
        , mGame(pAreaEntry->Game())
    {
        ASSERT(mpAreaEntry->ResourceType() == eArea);
    }

    void BuildDependencyList(std::list<CAssetID>& rAssetsOut, std::list<u32>& rLayerOffsetsOut, std::set<CAssetID> *pAudioGroupsOut = nullptr);
    void AddDependency(const CAssetID& rkID, std::list<CAssetID>& rOut, std::set<CAssetID> *pAudioGroupsOut);
};

#endif // DEPENDENCYLISTBUILDERS

