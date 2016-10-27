#ifndef CANIMSET_H
#define CANIMSET_H

#include "CAnimation.h"
#include "CAnimEventData.h"
#include "CSkeleton.h"
#include "CSkin.h"
#include "Core/Resource/CDependencyGroup.h"
#include "Core/Resource/CResource.h"
#include "Core/Resource/TResPtr.h"
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
    std::set<u32> UsedAnimationIndices;
};

struct SSetAnimation
{
    TString Name;
    TResPtr<CAnimation> pAnim;
    TResPtr<CAnimEventData> pEventData;
};

class CAnimSet : public CResource
{
    DECLARE_RESOURCE_TYPE(eAnimSet)
    friend class CAnimSetLoader;

    std::vector<SSetCharacter> mCharacters;
    std::vector<SSetAnimation> mAnimations;

public:
    CAnimSet(CResourceEntry *pEntry = 0) : CResource(pEntry) {}

    ~CAnimSet()
    {
        // note: in MP2, event data isn't a standalone resource, so it's owned by the animset; therefore we need to delete it manually
        if (Game() >= eEchoesDemo)
        {
            for (u32 iAnim = 0; iAnim < mAnimations.size(); iAnim++)
            {
                SSetAnimation& rAnim = mAnimations[iAnim];
                CAnimEventData *pEvents = rAnim.pEventData;
                ASSERT(pEvents && !pEvents->Entry());
                rAnim.pEventData = nullptr; // make sure TResPtr destructor doesn't attempt to access
                delete pEvents;
            }
        }
    }

    CDependencyTree* BuildDependencyTree() const
    {
        CDependencyTree *pTree = new CDependencyTree(ID());

        // Character dependencies
        for (u32 iChar = 0; iChar < mCharacters.size(); iChar++)
        {
            CSetCharacterDependency *pCharTree = CSetCharacterDependency::BuildTree(this, iChar);
            ASSERT(pCharTree);
            pTree->AddChild(pCharTree);
        }

        for (u32 iAnim = 0; iAnim < mAnimations.size(); iAnim++)
        {
            CSetAnimationDependency *pAnimTree = CSetAnimationDependency::BuildTree(this, iAnim);
            ASSERT(pAnimTree);
            pTree->AddChild(pAnimTree);
        }

        return pTree;
    }

    // Accessors
    inline u32 NumCharacters() const                        { return mCharacters.size(); }
    inline u32 NumAnimations() const                        { return mAnimations.size(); }
    inline const SSetCharacter* Character(u32 Index) const  { ASSERT(Index >= 0 && Index < NumCharacters()); return &mCharacters[Index]; }
    inline const SSetAnimation* Animation(u32 Index) const  { ASSERT(Index >= 0 && Index < NumAnimations()); return &mAnimations[Index]; }
};

#endif // CCHARACTERSET_H
