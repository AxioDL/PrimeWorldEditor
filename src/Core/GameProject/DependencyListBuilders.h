#ifndef DEPENDENCYLISTBUILDERS
#define DEPENDENCYLISTBUILDERS

#include "CDependencyTree.h"
#include "CPackage.h"
#include "CResourceEntry.h"
#include "Core/Resource/CDependencyGroup.h"
#include "Core/Resource/CWorld.h"

// ************ CPackageDependencyListBuilder ************
class CPackageDependencyListBuilder
{
    CPackage *mpPackage;
    TResPtr<CWorld> mpWorld;
    std::set<CAssetID> mPackageUsedAssets;
    std::set<CAssetID> mAreaUsedAssets;
    std::list<CAssetID> mScanIDs;
    bool mEnableDuplicates;
    bool mCurrentAreaHasDuplicates;
    bool mAddingScans;

public:
    CPackageDependencyListBuilder(CPackage *pPackage)
        : mpPackage(pPackage)
        , mCurrentAreaHasDuplicates(false)
        , mAddingScans(false)
    {}

    virtual void BuildDependencyList(bool AllowDuplicates, std::list<CAssetID>& rOut)
    {
        mEnableDuplicates = AllowDuplicates;

        for (u32 iCol = 0; iCol < mpPackage->NumCollections(); iCol++)
        {
            CResourceCollection *pCollection = mpPackage->CollectionByIndex(iCol);

            for (u32 iRes = 0; iRes < pCollection->NumResources(); iRes++)
            {
                const SNamedResource& rkRes = pCollection->ResourceByIndex(iRes);
                CResourceEntry *pEntry = gpResourceStore->FindEntry(rkRes.ID);

                if (pEntry)
                {
                    if (rkRes.Name.EndsWith("NODEPEND"))
                        rOut.push_back(rkRes.ID);

                    else
                    {
                        mScanIDs.clear();
                        mAddingScans = false;

                        if (rkRes.Type == "MLVL")
                        {
                            CResourceEntry *pWorldEntry = gpResourceStore->FindEntry(rkRes.ID);
                            ASSERT(pWorldEntry);
                            mpWorld = (CWorld*) pWorldEntry->Load();
                            ASSERT(mpWorld);
                        }

                        EvaluateDependencies(pEntry, rOut);

                        mAddingScans = true;
                        for (auto Iter = mScanIDs.begin(); Iter != mScanIDs.end(); Iter++)
                            AddDependency(pEntry, *Iter, rOut);
                    }
                }
            }
        }
    }

    inline void AddDependency(CResourceEntry *pCurEntry, const CAssetID& rkID, std::list<CAssetID>& rOut)
    {
        if (pCurEntry->ResourceType() == eDependencyGroup) return;
        CResourceEntry *pEntry = gpResourceStore->FindEntry(rkID);
        EResType Type = (pEntry ? pEntry->ResourceType() : eResource);

        // Defer scans to the end of the list. This is to accomodate the way the game loads SCANs; they are
        // loaded separately from everything else after the area itself is done loading.
        if (Type == eScan && !mAddingScans)
        {
            mScanIDs.push_back(rkID);
            return;
        }

        // Make sure entry exists + is valid
        if (pEntry && Type != eMidi && Type != eAudioGroupSet && Type != eWorld && (Type != eArea || pCurEntry->ResourceType() == eWorld))
        {
            if ( ( mCurrentAreaHasDuplicates && mAreaUsedAssets.find(rkID) == mAreaUsedAssets.end()) ||
                 (!mCurrentAreaHasDuplicates && mPackageUsedAssets.find(rkID) == mPackageUsedAssets.end()) )
            EvaluateDependencies(pEntry, rOut);
        }
    }

    void EvaluateDependencies(CResourceEntry *pEntry, std::list<CAssetID>& rOut)
    {
        // Toggle duplicates
        if (pEntry->ResourceType() == eArea && mEnableDuplicates)
        {
            mAreaUsedAssets.clear();

            for (u32 iArea = 0; iArea < mpWorld->NumAreas(); iArea++)
            {
                if (mpWorld->AreaResourceID(iArea) == pEntry->ID())
                {
                    mCurrentAreaHasDuplicates = mpWorld->DoesAreaAllowPakDuplicates(iArea);
                    break;
                }
            }
        }

        // Add dependencies
        CDependencyTree *pTree = pEntry->Dependencies();
        mPackageUsedAssets.insert(pTree->ID());
        mAreaUsedAssets.insert(pTree->ID());

        for (u32 iDep = 0; iDep < pTree->NumDependencies(); iDep++)
        {
            CAssetID ID = pTree->DependencyByIndex(iDep);
            AddDependency(pEntry, ID, rOut);
        }

        // Add area script dependencies
        if (pEntry->ResourceType() == eArea)
        {
            CAreaDependencyTree *pAreaTree = static_cast<CAreaDependencyTree*>(pTree);

            for (u32 iInst = 0; iInst < pAreaTree->NumScriptInstances(); iInst++)
            {
                CScriptInstanceDependencyTree *pInstTree = pAreaTree->ScriptInstanceByIndex(iInst);

                for (u32 iDep = 0; iDep < pInstTree->NumDependencies(); iDep++)
                {
                    CAssetID ID = pInstTree->DependencyByIndex(iDep);
                    AddDependency(pEntry, ID, rOut);
                }
            }
        }

        rOut.push_back(pTree->ID());
    }
};

// ************ CAreaDependencyListBuilder ************
class CAreaDependencyListBuilder
{
    CResourceEntry *mpAreaEntry;
    std::set<CAssetID> mBaseUsedAssets;
    std::set<CAssetID> mLayerUsedAssets;
    bool mIsPlayerActor;

public:
    CAreaDependencyListBuilder(CResourceEntry *pAreaEntry)
        : mpAreaEntry(pAreaEntry) {}

    virtual void BuildDependencyList(std::list<CAssetID>& rAssetsOut, std::list<u32>& rLayerOffsetsOut)
    {
        ASSERT(mpAreaEntry->ResourceType() == eArea);
        CAreaDependencyTree *pTree = static_cast<CAreaDependencyTree*>(mpAreaEntry->Dependencies());

        // Fill area base used assets set (don't actually add to list yet)
        for (u32 iDep = 0; iDep < pTree->NumDependencies(); iDep++)
            mBaseUsedAssets.insert(pTree->DependencyByIndex(iDep));

        // Get dependencies of each layer
        for (u32 iLyr = 0; iLyr < pTree->NumScriptLayers(); iLyr++)
        {
            mLayerUsedAssets.clear();
            rLayerOffsetsOut.push_back(rAssetsOut.size());

            bool IsLastLayer = (iLyr == pTree->NumScriptLayers() - 1);
            u32 StartIndex = pTree->ScriptLayerOffset(iLyr);
            u32 EndIndex = (IsLastLayer ? pTree->NumScriptInstances() : pTree->ScriptLayerOffset(iLyr + 1));

            for (u32 iInst = StartIndex; iInst < EndIndex; iInst++)
            {
                CScriptInstanceDependencyTree *pInstTree = pTree->ScriptInstanceByIndex(iInst);
                mIsPlayerActor = (pInstTree->ObjectType() == 0x4C);

                for (u32 iDep = 0; iDep < pInstTree->NumDependencies(); iDep++)
                {
                    CAssetID ID = pInstTree->DependencyByIndex(iDep);
                    AddDependency(mpAreaEntry, ID, rAssetsOut);
                }
            }
        }

        // Add base dependencies
        mBaseUsedAssets.clear();
        mLayerUsedAssets.clear();
        rLayerOffsetsOut.push_back(rAssetsOut.size());

        for (u32 iDep = 0; iDep < pTree->NumDependencies(); iDep++)
        {
            CAssetID ID = pTree->DependencyByIndex(iDep);
            AddDependency(mpAreaEntry, ID, rAssetsOut);
        }
    }

    void AddDependency(CResourceEntry *pCurEntry, const CAssetID& rkID, std::list<CAssetID>& rOut)
    {
        if (pCurEntry->ResourceType() == eDependencyGroup) return;
        CResourceEntry *pEntry = gpResourceStore->FindEntry(rkID);
        EResType Type = (pEntry ? pEntry->ResourceType() : eResource);

        // Make sure entry exists + is valid
        if (pEntry && Type != eMidi && Type != eAudioGroupSet && Type != eScan && Type != eWorld && Type != eArea)
        {
            if ( mBaseUsedAssets.find(rkID) == mBaseUsedAssets.end() && mLayerUsedAssets.find(rkID) == mLayerUsedAssets.end())
            {
                if (mIsPlayerActor && pEntry->ResourceType() == eAnimSet)
                    EvaluatePlayerActorAnimSet(pEntry, rOut);
                else
                    EvaluateDependencies(pEntry, rOut);
            }
        }
    }

    void EvaluateDependencies(CResourceEntry *pEntry, std::list<CAssetID>& rOut)
    {
        CDependencyTree *pTree = pEntry->Dependencies();
        mLayerUsedAssets.insert(pTree->ID());

        for (u32 iDep = 0; iDep < pTree->NumDependencies(); iDep++)
        {
            CAssetID ID = pTree->DependencyByIndex(iDep);
            AddDependency(pEntry, ID, rOut);
        }

        rOut.push_back(pTree->ID());
    }

    void EvaluatePlayerActorAnimSet(CResourceEntry *pEntry, std::list<CAssetID>& rOut)
    {
        // For PlayerActor animsets we want to include only the empty suit (char 5) in the dependency list. This is to
        // accomodate the dynamic loading the game does for PlayerActors to avoid having assets for suits the player
        // doesn't have in memory. We want common assets (animations, etc) in the list but not per-character assets.
        ASSERT(pEntry->ResourceType() == eAnimSet);
        CAnimSetDependencyTree *pTree = static_cast<CAnimSetDependencyTree*>(pEntry->Dependencies());
        mLayerUsedAssets.insert(pTree->ID());

        // Add empty suit dependencies
        ASSERT(pTree->NumCharacters() >= 7);
        u32 StartIdx = pTree->CharacterOffset(5);
        u32 EndIdx = pTree->CharacterOffset(6);

        for (u32 iDep = StartIdx; iDep < EndIdx; iDep++)
        {
            CAssetID ID = pTree->DependencyByIndex(iDep);
            AddDependency(pEntry, ID, rOut);
        }

        // Add common dependencies
        for (u32 iDep = 0; iDep < pTree->CharacterOffset(0); iDep++)
        {
            CAssetID ID = pTree->DependencyByIndex(iDep);
            AddDependency(pEntry, ID, rOut);
        }

        rOut.push_back(pTree->ID());
    }
};

#endif // DEPENDENCYLISTBUILDERS

