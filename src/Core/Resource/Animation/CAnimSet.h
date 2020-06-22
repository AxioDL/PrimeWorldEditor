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
#include <Common/BasicTypes.h>

#include <memory>
#include <set>
#include <vector>

// Animation structures
struct SAdditiveAnim
{
    uint32 AnimID;
    float FadeInTime;
    float FadeOutTime;
};

struct SAnimation
{
    TString Name;
    std::unique_ptr<IMetaAnimation> pMetaAnim;
};

struct STransition
{
    uint32 Unknown;
    uint32 AnimIdA;
    uint32 AnimIdB;
    std::unique_ptr<IMetaTransition> pMetaTrans;
};

struct SHalfTransition
{
    uint32 AnimID;
    std::unique_ptr<IMetaTransition> pMetaTrans;
};

// Character structures
enum class EOverlayType
{
    Frozen      = FOURCC('FRZN'),
    Hypermode   = FOURCC('HYPR'),
    Acid        = FOURCC('ACID'),
    XRay        = FOURCC('XRAY')
};

struct SOverlayModel
{
    EOverlayType Type;
    CAssetID ModelID;
    CAssetID SkinID;
};

struct SSetCharacter
{
    uint32 ID;
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
    std::set<uint32> UsedAnimationIndices;
};

class CAnimSet : public CResource
{
    DECLARE_RESOURCE_TYPE(AnimSet)
    friend class CAnimSetLoader;

    // Character Set
    std::vector<SSetCharacter> mCharacters;

    // Animation Set
    std::vector<CAnimPrimitive> mAnimPrimitives;
    std::vector<SAnimation> mAnimations;
    std::vector<STransition> mTransitions;
    std::unique_ptr<IMetaTransition> mpDefaultTransition;
    std::vector<SAdditiveAnim> mAdditiveAnims;
    float mDefaultAdditiveFadeIn = 0.0f;
    float mDefaultAdditiveFadeOut = 0.0f;
    std::vector<SHalfTransition> mHalfTransitions;
    std::vector<std::unique_ptr<CAnimEventData>> mAnimEvents; // note: these are for MP2, where event data isn't a standalone resource; these are owned by the animset

public:
    explicit CAnimSet(CResourceEntry *pEntry = nullptr)
        : CResource(pEntry)
    {}

    ~CAnimSet()
    {
        // For MP2, anim events need to be cleaned up manually
        for ([[maybe_unused]] const auto& event : mAnimEvents)
        {
            ASSERT(event != nullptr && !event->Entry());
        }
    }

    std::unique_ptr<CDependencyTree> BuildDependencyTree() const
    {
        auto pTree = std::make_unique<CDependencyTree>();

        // Character dependencies
        for (const auto& character : mCharacters)
        {
            auto pCharTree = CSetCharacterDependency::BuildTree(character);
            ASSERT(pCharTree);
            pTree->AddChild(std::move(pCharTree));
        }

        // Animation dependencies
        if (Game() <= EGame::Echoes)
        {
            for (uint32 iAnim = 0; iAnim < mAnimations.size(); iAnim++)
            {
                auto pAnimTree = CSetAnimationDependency::BuildTree(this, iAnim);
                ASSERT(pAnimTree);
                pTree->AddChild(std::move(pAnimTree));
            }
        }
        else if (Game() <= EGame::Corruption)
        {
            const SSetCharacter& rkChar = mCharacters[0];
            std::set<CAnimPrimitive> PrimitiveSet;

            // Animations
            for (const auto& anim : mAnimations)
            {
                anim.pMetaAnim->GetUniquePrimitives(PrimitiveSet);
            }

            if (auto* pAnimData = gpResourceStore->LoadResource<CSourceAnimData>(rkChar.AnimDataID))
                pAnimData->AddTransitionDependencies(pTree.get());

            for (const auto& prim : PrimitiveSet)
            {
                pTree->AddDependency(prim.Animation());
            }

            // Event sounds
            for (const auto& effect : rkChar.SoundEffects)
            {
                pTree->AddDependency(effect);
            }
        }
        else
        {
            const SSetCharacter& rkChar = mCharacters[0];

            for (const auto& dep : rkChar.DKDependencies)
                pTree->AddDependency(dep);
        }

        return pTree;
    }

    CAnimation* FindAnimationAsset(uint32 AnimID) const
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
        for (uint32 iAnim = 0; iAnim < mAnimPrimitives.size(); iAnim++)
            rPrimSet.insert(mAnimPrimitives[iAnim]);
    }

    // Accessors
    uint32 NumCharacters() const        { return mCharacters.size(); }
    uint32 NumAnimations() const        { return mAnimations.size(); }

    const SSetCharacter* Character(uint32 Index) const
    {
        ASSERT(Index >= 0 && Index < NumCharacters());
        return &mCharacters[Index];
    }

    const SAnimation* Animation(uint32 Index) const
    {
        ASSERT(Index >= 0 && Index < NumAnimations());
        return &mAnimations[Index];
    }

    CAnimEventData* AnimationEventData(uint32 Index) const
    {
        ASSERT(Index >= 0 && Index < NumAnimations());

        if (Game() <= EGame::Prime)
        {
            const CAnimPrimitive& rkPrim = mAnimPrimitives[Index];
            return rkPrim.Animation() ? rkPrim.Animation()->EventData() : nullptr;
        }

        else
        {
            return (Index < mAnimEvents.size() ? mAnimEvents[Index].get() : nullptr);
        }
    }
};

#endif // CCHARACTERSET_H
