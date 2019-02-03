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
    CResourceStore *mpStore;
    uint32 mLayerIndex;
    bool mIsInitialArea;
    bool mCurrentAreaAllowsDupes;

public:
    CCharacterUsageMap(CResourceStore *pStore)
        : mpStore(pStore), mLayerIndex(-1), mIsInitialArea(true), mCurrentAreaAllowsDupes(false)
    {}

    bool IsCharacterUsed(const CAssetID& rkID, uint32 CharacterIndex) const;
    bool IsAnimationUsed(const CAssetID& rkID, CSetAnimationDependency *pAnim) const;
    void FindUsagesForAsset(CResourceEntry *pEntry);
    void FindUsagesForArea(CWorld *pWorld, CResourceEntry *pEntry);
    void FindUsagesForArea(CWorld *pWorld, uint32 AreaIndex);
    void FindUsagesForLayer(CResourceEntry *pAreaEntry, uint32 LayerIndex);
    void Clear();
    void DebugPrintContents();

protected:
    void ParseDependencyNode(IDependencyNode *pNode);
};

// ************ CPackageDependencyListBuilder ************
class CPackageDependencyListBuilder
{
    const CPackage *mpkPackage;
    CResourceStore *mpStore;
    EGame mGame;
    TResPtr<CWorld> mpWorld;
    CAssetID mCurrentAnimSetID;
    CCharacterUsageMap mCharacterUsageMap;
    std::set<CAssetID> mPackageUsedAssets;
    std::set<CAssetID> mAreaUsedAssets;
    std::set<CAssetID> mUniversalAreaAssets;
    bool mEnableDuplicates;
    bool mCurrentAreaHasDuplicates;
    bool mIsUniversalAreaAsset;
    bool mIsPlayerActor;

public:
    CPackageDependencyListBuilder(const CPackage *pkPackage)
        : mpkPackage(pkPackage)
        , mGame(pkPackage->Project()->Game())
        , mpStore(pkPackage->Project()->ResourceStore())
        , mCharacterUsageMap(pkPackage->Project()->ResourceStore())
        , mCurrentAreaHasDuplicates(false)
        , mIsPlayerActor(false)
    {
    }

    void BuildDependencyList(bool AllowDuplicates, std::list<CAssetID>& rOut);
    void AddDependency(CResourceEntry *pCurEntry, const CAssetID& rkID, std::list<CAssetID>& rOut);
    void EvaluateDependencyNode(CResourceEntry *pCurEntry, IDependencyNode *pNode, std::list<CAssetID>& rOut);
    void FindUniversalAreaAssets();
};

// ************ CAreaDependencyListBuilder ************
class CAreaDependencyListBuilder
{
    CResourceEntry *mpAreaEntry;
    CResourceStore *mpStore;
    EGame mGame;
    CAssetID mCurrentAnimSetID;
    CCharacterUsageMap mCharacterUsageMap;
    std::set<CAssetID> mBaseUsedAssets;
    std::set<CAssetID> mLayerUsedAssets;
    bool mIsPlayerActor;

public:
    CAreaDependencyListBuilder(CResourceEntry *pAreaEntry)
        : mpAreaEntry(pAreaEntry)
        , mpStore(pAreaEntry->ResourceStore())
        , mGame(pAreaEntry->Game())
        , mCharacterUsageMap(pAreaEntry->ResourceStore())
    {
        ASSERT(mpAreaEntry->ResourceType() == EResourceType::Area);
    }

    void BuildDependencyList(std::list<CAssetID>& rAssetsOut, std::list<uint32>& rLayerOffsetsOut, std::set<CAssetID> *pAudioGroupsOut = nullptr);
    void AddDependency(const CAssetID& rkID, std::list<CAssetID>& rOut, std::set<CAssetID> *pAudioGroupsOut);
    void EvaluateDependencyNode(CResourceEntry *pCurEntry, IDependencyNode *pNode, std::list<CAssetID>& rOut, std::set<CAssetID> *pAudioGroupsOut);
};

// ************ CAssetDependencyListBuilder ************
//@todo merge with CAreaDependencyListBuilder; code is very similar
class CAssetDependencyListBuilder
{
    CResourceEntry* mpResourceEntry;
    CCharacterUsageMap mCharacterUsageMap;
    std::set<CAssetID> mUsedAssets;
    CAssetID mCurrentAnimSetID;

public:
    CAssetDependencyListBuilder(CResourceEntry* pEntry)
        : mpResourceEntry(pEntry)
        , mCharacterUsageMap(pEntry->ResourceStore())
    {}

    void BuildDependencyList(std::vector<CAssetID>& OutAssets);
    void AddDependency(const CAssetID& kID, std::vector<CAssetID>& Out);
    void EvaluateDependencyNode(CResourceEntry* pCurEntry, IDependencyNode* pNode, std::vector<CAssetID>& Out);
};

#endif // DEPENDENCYLISTBUILDERS

