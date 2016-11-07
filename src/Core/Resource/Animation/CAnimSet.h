#ifndef CANIMSET_H
#define CANIMSET_H

#include "CAnimation.h"
#include "CAnimEventData.h"
#include "CSkeleton.h"
#include "CSkin.h"
#include "IMetaAnimation.h"
#include "IMetaTransition.h"
#include "Core/Resource/CDependencyGroup.h"
#include "Core/Resource/CResource.h"
#include "Core/Resource/TResPtr.h"
#include "Core/Resource/Model/CModel.h"
#include <Common/types.h>

#include <vector>

// Animation structures
struct SAdditiveAnim
{
    u32 AnimID;
    float FadeInTime;
    float FadeOutTime;
};

struct SAnimation
{
    TString Name;
    IMetaAnimation *pMetaAnim;
};

struct STransition
{
    u32 Unknown;
    u32 AnimIdA;
    u32 AnimIdB;
    IMetaTransition *pMetaTrans;
};

struct SHalfTransition
{
    u32 AnimID;
    IMetaTransition *pMetaTrans;
};

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
    CAssetID SpatialPrimitives;
    std::set<u32> UsedAnimationIndices;
};

class CAnimSet : public CResource
{
    DECLARE_RESOURCE_TYPE(eAnimSet)
    friend class CAnimSetLoader;

    // Character Set
    std::vector<SSetCharacter> mCharacters;

    // Animation Set
    std::vector<CAnimPrimitive> mAnimPrimitives;
    std::vector<SAnimation> mAnimations;
    std::vector<STransition> mTransitions;
    IMetaTransition *mpDefaultTransition;
    std::vector<SAdditiveAnim> mAdditiveAnims;
    float mDefaultAdditiveFadeIn;
    float mDefaultAdditiveFadeOut;
    std::vector<SHalfTransition> mHalfTransitions;
    std::vector<CAnimEventData*> mAnimEvents; // note: these are for MP2, where event data isn't a standalone resource; these are owned by the animset

public:
    CAnimSet(CResourceEntry *pEntry = 0) : CResource(pEntry) {}

    ~CAnimSet()
    {
        // For MP2, anim events need to be cleaned up manually
        if (Game() >= eEchoesDemo)
        {
            for (u32 iEvent = 0; iEvent < mAnimEvents.size(); iEvent++)
            {
                ASSERT(mAnimEvents[iEvent] && !mAnimEvents[iEvent]->Entry());
                delete mAnimEvents[iEvent];
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

    CAnimation* FindAnimationAsset(u32 AnimID) const
    {
        if (AnimID >= 0 && AnimID < mAnimPrimitives.size())
        {
            CAnimPrimitive Prim = mAnimPrimitives[AnimID];
            return Prim.Animation();
        }

        return nullptr;
    }

    // Accessors
    inline u32 NumCharacters() const        { return mCharacters.size(); }
    inline u32 NumAnimations() const        { return mAnimations.size(); }

    inline const SSetCharacter* Character(u32 Index) const
    {
        ASSERT(Index >= 0 && Index < NumCharacters());
        return &mCharacters[Index];
    }

    inline const SAnimation* Animation(u32 Index) const
    {
        ASSERT(Index >= 0 && Index < NumAnimations());
        return &mAnimations[Index];
    }

    CAnimEventData* AnimationEventData(u32 Index) const
    {
        ASSERT(Index >= 0 && Index < NumAnimations());

        if (Game() <= ePrime)
        {
            const CAnimPrimitive& rkPrim = mAnimPrimitives[Index];
            return rkPrim.Animation() ? rkPrim.Animation()->EventData() : nullptr;
        }

        else
        {
            return (Index < mAnimEvents.size() ? mAnimEvents[Index] : nullptr);
        }
    }
};

#endif // CCHARACTERSET_H
