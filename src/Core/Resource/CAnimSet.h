#ifndef CANIMSET_H
#define CANIMSET_H

#include "TResPtr.h"
#include "CAnimation.h"
#include "CAnimEventData.h"
#include "CDependencyGroup.h"
#include "CResource.h"
#include "CSkeleton.h"
#include "CSkin.h"
#include "Core/Resource/Model/CModel.h"
#include <Common/types.h>

#include <vector>

struct SSetCharacter
{
    TString Name;
    TResPtr<CModel> pModel;
    TResPtr<CSkin> pSkin;
    TResPtr<CSkeleton> pSkeleton;

    std::vector<CAssetID> GenericParticles;
    std::vector<CAssetID> ElectricParticles;
    std::vector<CAssetID> SwooshParticles;
    std::vector<CAssetID> SpawnParticles;
    std::vector<CAssetID> EffectParticles;
    CAssetID IceModel;
    CAssetID IceSkin;
};

class CAnimSet : public CResource
{
    DECLARE_RESOURCE_TYPE(eAnimSet)
    friend class CAnimSetLoader;

    std::vector<SSetCharacter> mCharacters;

    struct SAnimation
    {
        TString Name;
        TResPtr<CAnimation> pAnim;
    };
    std::vector<SAnimation> mAnims;
    std::vector<CAnimEventData*> mEventDependencies;

public:
    CAnimSet(CResourceEntry *pEntry = 0) : CResource(pEntry) {}

    ~CAnimSet()
    {
        for (u32 iEvnt = 0; iEvnt < mEventDependencies.size(); iEvnt++)
        {
            ASSERT(!mEventDependencies[iEvnt]->Entry());
            delete mEventDependencies[iEvnt];
        }
    }

    u32 NumNodes() const                { return mCharacters.size(); }
    TString NodeName(u32 Index)         { if (Index >= mCharacters.size()) Index = 0; return mCharacters[Index].Name; }
    CModel* NodeModel(u32 Index)        { if (Index >= mCharacters.size()) Index = 0; return mCharacters[Index].pModel; }
    CSkin* NodeSkin(u32 Index)          { if (Index >= mCharacters.size()) Index = 0; return mCharacters[Index].pSkin; }
    CSkeleton* NodeSkeleton(u32 Index)  { if (Index >= mCharacters.size()) Index = 0; return mCharacters[Index].pSkeleton; }

    u32 NumAnims() const                { return mAnims.size(); }
    CAnimation* Animation(u32 Index)    { if (Index >= mAnims.size()) Index = 0; return mAnims[Index].pAnim; }
    TString AnimName(u32 Index)         { if (Index >= mAnims.size()) Index = 0; return mAnims[Index].Name; }

    CDependencyTree* BuildDependencyTree() const
    {
        CAnimSetDependencyTree *pTree = new CAnimSetDependencyTree(ID());
        std::set<CAssetID> BaseUsedSet;

        // Base dependencies
        for (u32 iAnim = 0; iAnim < mAnims.size(); iAnim++)
        {
            CAnimation *pAnim = mAnims[iAnim].pAnim;

            if (pAnim)
            {
                pTree->AddDependency(mAnims[iAnim].pAnim);
                BaseUsedSet.insert(pAnim->ID());
            }
        }

        for (u32 iEvnt = 0; iEvnt < mEventDependencies.size(); iEvnt++)
        {
            CAnimEventData *pData = mEventDependencies[iEvnt];

            for (u32 iEvt = 0; iEvt < pData->NumEvents(); iEvt++)
            {
                CAssetID ID = pData->EventAssetRef(iEvt);
                u32 CharIdx = pData->EventCharacterIndex(iEvt);
                pTree->AddEventDependency(ID, CharIdx);
                BaseUsedSet.insert(ID);
            }
        }

        // Character dependencies
        for (u32 iNode = 0; iNode < mCharacters.size(); iNode++)
            pTree->AddCharacter(&mCharacters[iNode], BaseUsedSet);

        return pTree;
    }
};

#endif // CCHARACTERSET_H
