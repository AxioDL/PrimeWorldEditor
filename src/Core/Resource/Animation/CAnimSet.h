#ifndef CANIMSET_H
#define CANIMSET_H

#include "CAnimation.h"
#include "CAnimEventData.h"
#include "CSkeleton.h"
#include "CSkin.h"
#include "CSourceAnimData.h"
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

// Character structures
enum EOverlayType
{
    eOT_Frozen = FOURCC('FRZN'),
    eOT_Hypermode = FOURCC('HYPR'),
    eOT_Acid = FOURCC('ACID'),
    eOT_XRay = FOURCC('XRAY')
};

struct SOverlayModel
{
    EOverlayType Type;
    CAssetID ModelID;
    CAssetID SkinID;
};

struct SSetCharacter
{
    u32 ID;
    TString Name;
    TResPtr<CModel> pModel;
    TResPtr<CSkin> pSkin;
    TResPtr<CSkeleton> pSkeleton;
    std::vector<SOverlayModel> OverlayModels;
    CAssetID AnimDataID;
    CAssetID CollisionPrimitivesID;

    std::vector<CAssetID> GenericParticles;
    std::vector<CAssetID> ElectricParticles;
    std::vector<CAssetID> SwooshParticles;
    std::vector<CAssetID> SpawnParticles;
    std::vector<CAssetID> EffectParticles;
    std::vector<CAssetID> SoundEffects;
    std::vector<CAssetID> DKDependencies;
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
    CAnimSet(CResourceEntry *pEntry = 0)
        : CResource(pEntry)
        , mpDefaultTransition(nullptr)
    {}

    ~CAnimSet()
    {
        for (u32 iAnim = 0; iAnim < mAnimations.size(); iAnim++)
            delete mAnimations[iAnim].pMetaAnim;

        for (u32 iTrans = 0; iTrans < mTransitions.size(); iTrans++)
            delete mTransitions[iTrans].pMetaTrans;

        for (u32 iHalf = 0; iHalf < mHalfTransitions.size(); iHalf++)
            delete mHalfTransitions[iHalf].pMetaTrans;

        delete mpDefaultTransition;

        // For MP2, anim events need to be cleaned up manually
        for (u32 iEvent = 0; iEvent < mAnimEvents.size(); iEvent++)
        {
            ASSERT(mAnimEvents[iEvent] && !mAnimEvents[iEvent]->Entry());
            delete mAnimEvents[iEvent];
        }
    }

    CDependencyTree* BuildDependencyTree() const
    {
        CDependencyTree *pTree = new CDependencyTree();

        // Character dependencies
        for (u32 iChar = 0; iChar < mCharacters.size(); iChar++)
        {
            CSetCharacterDependency *pCharTree = CSetCharacterDependency::BuildTree( mCharacters[iChar] );
            ASSERT(pCharTree);
            pTree->AddChild(pCharTree);
        }

        // Animation dependencies
        if (Game() <= EGame::Echoes)
        {
            for (u32 iAnim = 0; iAnim < mAnimations.size(); iAnim++)
            {
                CSetAnimationDependency *pAnimTree = CSetAnimationDependency::BuildTree(this, iAnim);
                ASSERT(pAnimTree);
                pTree->AddChild(pAnimTree);
            }
        }

        else if (Game() <= EGame::Corruption)
        {
            const SSetCharacter& rkChar = mCharacters[0];
            std::set<CAnimPrimitive> PrimitiveSet;

            // Animations
            for (u32 iAnim = 0; iAnim < mAnimations.size(); iAnim++)
            {
                const SAnimation& rkAnim = mAnimations[iAnim];
                rkAnim.pMetaAnim->GetUniquePrimitives(PrimitiveSet);
            }

            CSourceAnimData *pAnimData = gpResourceStore->LoadResource<CSourceAnimData>(rkChar.AnimDataID);
            if (pAnimData)
                pAnimData->AddTransitionDependencies(pTree);

            for (auto Iter = PrimitiveSet.begin(); Iter != PrimitiveSet.end(); Iter++)
            {
                const CAnimPrimitive& rkPrim = *Iter;
                pTree->AddDependency(rkPrim.Animation());
            }

            // Event sounds
            for (u32 iSound = 0; iSound < rkChar.SoundEffects.size(); iSound++)
            {
                pTree->AddDependency(rkChar.SoundEffects[iSound]);
            }
        }

        else
        {
            const SSetCharacter& rkChar = mCharacters[0];

            for (u32 iDep = 0; iDep < rkChar.DKDependencies.size(); iDep++)
                pTree->AddDependency(rkChar.DKDependencies[iDep]);
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

    void GetUniquePrimitives(std::set<CAnimPrimitive>& rPrimSet) const
    {
        for (u32 iAnim = 0; iAnim < mAnimPrimitives.size(); iAnim++)
            rPrimSet.insert(mAnimPrimitives[iAnim]);
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

        if (Game() <= EGame::Prime)
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
