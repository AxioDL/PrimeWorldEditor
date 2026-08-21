#include "Core/Resource/Factory/CAnimSetLoader.h"

#include "Core/GameProject/CResourceStore.h"
#include "Core/Resource/Animation/CAnimSet.h"
#include "Core/Resource/Animation/CSourceAnimData.h"
#include "Core/Resource/Factory/CAnimEventLoader.h"

#include <Common/Log.h>

CAnimSetLoader::CAnimSetLoader() = default;

void CAnimSetLoader::LoadCorruptionCHAR(IInputStream& rCHAR)
{
    auto* const resourceStore = pSet->Entry()->ResourceStore();

    SSetCharacter& rChar = pSet->mCharacters.emplace_back();

    // Character Header
    rChar.ID = rCHAR.ReadU8();
    rChar.Name = rCHAR.ReadString();
    rChar.pModel = resourceStore->LoadResource<CModel>(CAssetID(rCHAR.ReadU64()));
    rChar.pSkin = resourceStore->LoadResource<CSkin>(CAssetID(rCHAR.ReadU64()));

    const auto NumOverlays = rCHAR.ReadU32();

    rChar.OverlayModels.reserve(NumOverlays);
    for (uint32_t i = 0; i < NumOverlays; i++)
    {
        rChar.OverlayModels.push_back({
            .Type = static_cast<EOverlayType>(rCHAR.ReadU32()),
            .ModelID = CAssetID(rCHAR, EIDLength::k64Bit),
            .SkinID = CAssetID(rCHAR, EIDLength::k64Bit),
        });
    }

    rChar.pSkeleton = resourceStore->LoadResource<CSkeleton>(CAssetID(rCHAR.ReadU64()));
    rChar.AnimDataID = CAssetID(rCHAR, EIDLength::k64Bit);

    // PAS Database
    LoadPASDatabase(rCHAR);

    // Particle Resource Data
    LoadParticleResourceData(rCHAR, &rChar, 10);

    // Events
    const auto NumEventSets = rCHAR.ReadU32();

    pSet->mAnimEvents.reserve(NumEventSets);
    for (uint32_t i = 0; i < NumEventSets; i++)
    {
        pSet->mAnimEvents.push_back(CAnimEventLoader::LoadCorruptionCharacterEventSet(rCHAR, resourceStore));
    }

    // Animations
    const auto NumAnimations = rCHAR.ReadU32();

    pSet->mAnimations.reserve(NumAnimations);
    for (uint32_t i = 0; i < NumAnimations; i++)
    {
        pSet->mAnimations.push_back({
            .Name = rCHAR.ReadString(),
            .pMetaAnim = CMetaAnimFactory::LoadFromStream(resourceStore, rCHAR, mGame),
        });
    }

    // Animation Bounds
    const auto NumAnimationBounds = rCHAR.ReadU32();
    rCHAR.Skip(NumAnimationBounds * 0x20);
    rCHAR.Skip(1);

    // Bool Array
    const auto BoolArraySize = rCHAR.ReadU32();
    rCHAR.Skip(BoolArraySize);

    // Collision Primitives
    const auto NumPrimitiveSets = rCHAR.ReadU32();

    for (uint32_t SetIdx = 0; SetIdx < NumPrimitiveSets; SetIdx++)
    {
        rCHAR.ReadString();
        const auto NumPrimitives = rCHAR.ReadU32();

        for (uint32_t PrimIdx = 0; PrimIdx < NumPrimitives; PrimIdx++)
        {
            rCHAR.Skip(0x34);
            rCHAR.ReadString();
            rCHAR.Skip(4);
        }
    }

    // Sound Resources
    const auto NumSounds = rCHAR.ReadU32();

    rChar.SoundEffects.reserve(NumSounds);
    for (uint32_t i = 0; i < NumSounds; i++)
    {
        const CAssetID SoundID(rCHAR, EIDLength::k64Bit);
        rChar.SoundEffects.push_back(SoundID);
    }

    ProcessPrimitives();
}

void CAnimSetLoader::LoadReturnsCHAR(IInputStream& rCHAR)
{
    auto* const resourceStore = pSet->Entry()->ResourceStore();

    rCHAR.Skip(0x14);
    const auto Flag = rCHAR.ReadU8();
    rCHAR.Skip(1);

    SSetCharacter& rChar = pSet->mCharacters.emplace_back();

    // Character Header
    rChar.ID = 0;
    rChar.Name = rCHAR.ReadString();
    rChar.pSkeleton = resourceStore->LoadResource<CSkeleton>(CAssetID(rCHAR.ReadU64()));
    rChar.CollisionPrimitivesID = CAssetID(rCHAR.ReadU64());

    const auto NumModels = rCHAR.ReadU32();

    for (uint32_t ModelIdx = 0; ModelIdx < NumModels; ModelIdx++)
    {
        rCHAR.ReadString();
        const CAssetID ModelID(rCHAR, EGame::DKCReturns);
        const CAssetID SkinID(rCHAR, EGame::DKCReturns);
        rCHAR.Skip(0x18);

        if (ModelIdx == 0)
        {
            rChar.pModel = resourceStore->LoadResource<CModel>(ModelID);
            rChar.pSkin = resourceStore->LoadResource<CSkin>(SkinID);
        }
        else
        {
            rChar.DKDependencies.push_back(ModelID);
            rChar.DKDependencies.push_back(SkinID);
        }
    }

    // Animations
    const auto NumAnims = rCHAR.ReadU32();

    pSet->mAnimations.reserve(NumAnims);
    for (uint32_t AnimIdx = 0; AnimIdx < NumAnims; AnimIdx++)
    {
        TString AnimName = rCHAR.ReadString();
        const CAssetID AnimID(rCHAR, EGame::DKCReturns);
        rCHAR.Skip(0x25);
        rChar.DKDependencies.push_back(AnimID);

        // small hack - create a meta-anim for it so we can generate asset names for the ANIM files correctly
        SAnimation Anim;
        Anim.pMetaAnim = std::make_unique<CMetaAnimPlay>(CAnimPrimitive(resourceStore, AnimID, AnimIdx, AnimName), 0.f, CCharAnimTime::EType::NonZero);
        Anim.Name = std::move(AnimName);
        pSet->mAnimations.push_back(std::move(Anim));
    }

    // The only other thing we care about right now is the dependency list. If this file doesn't have a dependency list, exit out.
    if ((Flag & 0x10) == 0)
        return;

    // Anim ID Map
    if ((Flag & 0x20) != 0)
    {
        const auto NumIDs = rCHAR.ReadU32();
        rCHAR.Skip(NumIDs * 4);
    }

    // Transitions
    if ((Flag & 0x80) != 0)
    {
        const auto NumAdditiveAnims = rCHAR.ReadU32();
        rCHAR.Skip(NumAdditiveAnims * 0x10);

        const auto NumTransitionTypes = rCHAR.ReadU32();

        for (uint32_t TypeIdx = 0; TypeIdx < NumTransitionTypes; TypeIdx++)
        {
            const auto Type = rCHAR.ReadU16();

            switch (Type)
            {
            case 0:
                break;
            case 1:
            case 2:
                rCHAR.Skip(9);
                break;
            case 3:
                rCHAR.Skip(0xC);
                break;
            default:
                NLog::Error("{} [0x{:X}]: Invalid transition type: {}", rCHAR.GetSourceString(), rCHAR.Tell() - 2, Type);
                return;
            }
        }

        const auto NumFullTransitions = rCHAR.ReadU32();
        rCHAR.Skip(NumFullTransitions * 0xC);

        const auto NumHalfTransitions = rCHAR.ReadU32();
        rCHAR.Skip(NumHalfTransitions * 0x8);

        rCHAR.Skip(0x8);
    }

    // Transform Bits
    if ((Flag & 0x40) != 0)
    {
        const auto NumTransformBits = rCHAR.ReadU32();
        rCHAR.Skip(NumTransformBits);
    }

    const auto NumUnknown = rCHAR.ReadU32();
    rCHAR.Skip(NumUnknown * 4);

    // Skel Joint Sets
    const auto NumSkelJointSets = rCHAR.ReadU32();

    for (uint32_t SetIdx = 0; SetIdx < NumSkelJointSets; SetIdx++)
    {
        rCHAR.Skip(4);
        const auto NumUnknown2 = rCHAR.ReadU32();
        rCHAR.Skip(0x20 + NumUnknown2);
    }

    // Resources
    if ((Flag & 0x10) != 0)
    {
        // Don't need the extensions
        const auto NumExtensions = rCHAR.ReadU32();
        rCHAR.Skip(NumExtensions * 4);

        const auto NumResources = rCHAR.ReadU32();

        rChar.DKDependencies.reserve(NumResources);
        for (uint32_t ResIdx = 0; ResIdx < NumResources; ResIdx++)
        {
            const CAssetID ResID(rCHAR, EGame::DKCReturns);
            rCHAR.Skip(3);
            rChar.DKDependencies.push_back(ResID);
        }
    }

    ProcessPrimitives();
}

void CAnimSetLoader::LoadPASDatabase(IInputStream& rPAS4)
{
    // For now, just parse the data; don't store it
    [[maybe_unused]] const auto Magic = rPAS4.ReadFourCC();
    const auto AnimStateCount = rPAS4.ReadU32();
    rPAS4.Seek(0x4, SEEK_CUR); // Skipping default anim state
    ASSERT(Magic == CFourCC("PAS4"));

    for (uint32_t iState = 0; iState < AnimStateCount; iState++)
    {
        rPAS4.Seek(0x4, SEEK_CUR); // Skipping unknown value
        const auto ParmInfoCount = rPAS4.ReadU32();
        const auto AnimInfoCount = rPAS4.ReadU32();

        uint32_t Skip = 0;
        for (uint32_t iParm = 0; iParm < ParmInfoCount; iParm++)
        {
            const auto Type = rPAS4.ReadU32();
            rPAS4.Seek(0x8, SEEK_CUR);

            switch (Type) {
            case 0: // Int32
            case 1: // Uint32
            case 2: // Real32
            case 4: // Enum
                rPAS4.Seek(0x8, SEEK_CUR);
                Skip += 4;
                break;
            case 3: // Bool
                rPAS4.Seek(0x2, SEEK_CUR);
                Skip++;
                break;
            default:
                break;
            }
        }

        for (uint32_t iInfo = 0; iInfo < AnimInfoCount; iInfo++)
            rPAS4.Seek(0x4 + Skip, SEEK_CUR);
    }
}

void CAnimSetLoader::LoadParticleResourceData(IInputStream& rFile, SSetCharacter *pChar, uint16_t CharVersion)
{
    const auto ParticleCount = rFile.ReadU32();
    pChar->GenericParticles.reserve(ParticleCount);

    for (uint32_t iPart = 0; iPart < ParticleCount; iPart++)
        pChar->GenericParticles.emplace_back(rFile, mGame);

    const auto SwooshCount = rFile.ReadU32();
    pChar->SwooshParticles.reserve(SwooshCount);

    for (uint32_t iSwoosh = 0; iSwoosh < SwooshCount; iSwoosh++)
        pChar->SwooshParticles.emplace_back(rFile, mGame);

    if (CharVersion >= 6 && mGame <= EGame::Echoes)
        rFile.Seek(0x4, SEEK_CUR);

    const auto ElectricCount = rFile.ReadU32();
    pChar->ElectricParticles.reserve(ElectricCount);

    for (uint32_t iElec = 0; iElec < ElectricCount; iElec++)
        pChar->ElectricParticles.emplace_back(rFile, mGame);

    if (mGame >= EGame::Echoes)
    {
        const auto SpawnCount = rFile.ReadU32();
        pChar->SpawnParticles.reserve(SpawnCount);

        for (uint32_t iSpawn = 0; iSpawn < SpawnCount; iSpawn++)
            pChar->SpawnParticles.emplace_back(rFile, mGame);
    }

    rFile.Seek(0x4, SEEK_CUR);
    if (mGame >= EGame::Echoes)
        rFile.Seek(0x4, SEEK_CUR);
}

void CAnimSetLoader::LoadAnimationSet(IInputStream& rANCS)
{
    auto* const resourceStore = pSet->Entry()->ResourceStore();

    const auto Version = rANCS.ReadU16();

    // Animations
    const auto NumAnims = rANCS.ReadU32();

    pSet->mAnimations.reserve(NumAnims);
    for (uint32_t i = 0; i < NumAnims; i++)
    {
        pSet->mAnimations.push_back({
            .Name = rANCS.ReadString(),
            .pMetaAnim = CMetaAnimFactory::LoadFromStream(resourceStore, rANCS, mGame),
        });
    }

    // Transitions
    const auto NumTransitions = rANCS.ReadU32();

    pSet->mTransitions.reserve(NumTransitions);
    for (uint32_t i = 0; i < NumTransitions; i++)
    {
        pSet->mTransitions.push_back({
            .ID = rANCS.ReadU32(),
            .AnimIdA = rANCS.ReadU32(),
            .AnimIdB = rANCS.ReadU32(),
            .pMetaTrans = CMetaTransFactory::LoadFromStream(resourceStore, rANCS, mGame),
        });
    }

    pSet->mpDefaultTransition = CMetaTransFactory::LoadFromStream(resourceStore, rANCS, mGame);

    // Additive Animations
    const auto NumAdditive = rANCS.ReadU32();

    pSet->mAdditiveAnims.reserve(NumAdditive);
    for (uint32_t i = 0; i < NumAdditive; i++)
    {
        pSet->mAdditiveAnims.push_back({
            .AnimID = rANCS.ReadU32(),
            .FadeInTime = rANCS.ReadF32(),
            .FadeOutTime = rANCS.ReadF32(),
        });
    }

    pSet->mDefaultAdditiveFadeIn = rANCS.ReadF32();
    pSet->mDefaultAdditiveFadeOut = rANCS.ReadF32();

    // Half Transitions
    if (Version > 2)
    {
        const auto NumHalfTransitions = rANCS.ReadU32();

        pSet->mHalfTransitions.reserve(NumHalfTransitions);
        for (uint32_t i = 0; i < NumHalfTransitions; i++)
        {
            pSet->mHalfTransitions.push_back({
                .AnimID = rANCS.ReadU32(),
                .pMetaTrans = CMetaTransFactory::LoadFromStream(resourceStore, rANCS, mGame),
            });
        }
    }

    // Skipping MP1 ANIM asset list
    // Events
    if (mGame >= EGame::EchoesDemo)
    {
        const auto EventDataCount = rANCS.ReadU32();
        pSet->mAnimEvents.reserve(EventDataCount);
        ASSERT(EventDataCount == NumAnims);

        for (uint32_t i = 0; i < EventDataCount; i++)
        {
            pSet->mAnimEvents.push_back(CAnimEventLoader::LoadAnimSetEvents(rANCS, pSet->Entry()->ResourceStore()));
        }
    }
}

void CAnimSetLoader::ProcessPrimitives()
{
    // Find all unique anim primitives
    std::set<CAnimPrimitive> UniquePrimitives;

    for (const auto& anim : pSet->mAnimations)
        anim.pMetaAnim->GetUniquePrimitives(UniquePrimitives);

    for (const auto& transition : pSet->mTransitions)
        transition.pMetaTrans->GetUniquePrimitives(UniquePrimitives);

    if (pSet->mpDefaultTransition)
        pSet->mpDefaultTransition->GetUniquePrimitives(UniquePrimitives);

    for (const auto& halfTransition : pSet->mHalfTransitions)
        halfTransition.pMetaTrans->GetUniquePrimitives(UniquePrimitives);

    if (mGame == EGame::CorruptionProto || mGame == EGame::Corruption)
    {
        auto* pAnimData = pSet->Entry()->ResourceStore()->LoadResource<CSourceAnimData>(pSet->mCharacters[0].AnimDataID);

        if (pAnimData != nullptr)
            pAnimData->GetUniquePrimitives(UniquePrimitives);
    }

    // Copy anim primitives into the animset
    for (const auto& primitive : UniquePrimitives)
    {
        const auto ID = primitive.ID();

        if (ID >= pSet->mAnimPrimitives.size())
            pSet->mAnimPrimitives.resize(ID + 1);

        pSet->mAnimPrimitives[ID] = primitive;
    }

    // Add used animation indices from the animset to the character's list
    if (mGame <= EGame::Echoes)
    {
        // Add animations referenced by default transition
        if (pSet->mpDefaultTransition)
        {
            std::set<CAnimPrimitive> DefaultTransPrimitives;
            pSet->mpDefaultTransition->GetUniquePrimitives(DefaultTransPrimitives);

            for (auto& character : pSet->mCharacters)
            {
                for (const auto& primitive : DefaultTransPrimitives)
                {
                    character.UsedAnimationIndices.insert(primitive.ID());
                }
            }
        }

        // Add animations referenced by used transitions
        for (auto& character : pSet->mCharacters)
        {
            bool AddedNewAnims = true;

            // Loop this until we run out of new animations. This is in case any animations
            // referenced by any transitions are also referenced by earlier transitions.
            while (AddedNewAnims)
            {
                AddedNewAnims = false;

                for (auto& transition : pSet->mTransitions)
                {
                    if (!character.UsedAnimationIndices.contains(transition.AnimIdA) ||
                        !character.UsedAnimationIndices.contains(transition.AnimIdB))
                    {
                        continue;
                    }

                    std::set<CAnimPrimitive> Primitives;
                    transition.pMetaTrans->GetUniquePrimitives(Primitives);

                    for (const auto& primitive : Primitives)
                    {
                        if (!character.UsedAnimationIndices.contains(primitive.ID()))
                        {
                            character.UsedAnimationIndices.insert(primitive.ID());
                            AddedNewAnims = true;
                        }
                    }
                }

                for (SHalfTransition& trans : pSet->mHalfTransitions)
                {
                    if (!character.UsedAnimationIndices.contains(trans.AnimID))
                        continue;

                    std::set<CAnimPrimitive> Primitives;
                    trans.pMetaTrans->GetUniquePrimitives(Primitives);

                    for (const auto& primitive : Primitives)
                    {
                        if (!character.UsedAnimationIndices.contains(primitive.ID()))
                        {
                            character.UsedAnimationIndices.insert(primitive.ID());
                            AddedNewAnims = true;
                        }
                    }
                }
            }
        }
    }
}

// ************ STATIC ************
std::unique_ptr<CAnimSet> CAnimSetLoader::LoadANCS(IInputStream& rANCS, CResourceEntry *pEntry)
{
    if (!rANCS.IsValid())
        return nullptr;

    const auto Magic = rANCS.ReadU32();
    if (Magic != 0x00010001)
    {
        NLog::Error("{}: Invalid ANCS magic: 0x{:08X}", rANCS.GetSourceString(), Magic);
        return nullptr;
    }

    auto* const resourceStore = pEntry->ResourceStore();
    auto ptr = std::make_unique<CAnimSet>(pEntry);

    CAnimSetLoader Loader;
    Loader.pSet = ptr.get();
    Loader.mGame = pEntry->Game();

    const auto NodeCount = rANCS.ReadU32();
    Loader.pSet->mCharacters.resize(NodeCount);

    for (uint32_t iNode = 0; iNode < NodeCount; iNode++)
    {
        SSetCharacter *pChar = &Loader.pSet->mCharacters[iNode];

        pChar->ID = rANCS.ReadU32();
        const auto CharVersion = rANCS.ReadU16();
        if (iNode == 0 && Loader.mGame == EGame::Invalid)
        {
            Loader.mGame = (CharVersion == 0xA) ? EGame::Echoes : EGame::Prime;
        }
        pChar->Name = rANCS.ReadString();
        pChar->pModel = resourceStore->LoadResource<CModel>(rANCS.ReadU32());
        pChar->pSkin = resourceStore->LoadResource<CSkin>(rANCS.ReadU32());
        pChar->pSkeleton = resourceStore->LoadResource<CSkeleton>(rANCS.ReadU32());
        if (pChar->pModel != nullptr)
            pChar->pModel->SetSkin(pChar->pSkin);

        // Unfortunately that's all that's actually supported at the moment. Hope to expand later.
        // Since there's no size value I have to actually read the rest of the node to reach the next one
        const auto AnimCount = rANCS.ReadU32();

        for (uint32_t iAnim = 0; iAnim < AnimCount; iAnim++)
        {
            rANCS.Seek(0x4, SEEK_CUR);
            if (Loader.mGame == EGame::Prime)
                rANCS.Seek(0x1, SEEK_CUR);
            rANCS.ReadString();
        }

        // PAS Database
        Loader.LoadPASDatabase(rANCS);

        // Particles
        Loader.LoadParticleResourceData(rANCS, pChar, CharVersion);

        const auto AnimCount2 = rANCS.ReadU32();
        for (uint32_t iAnim = 0; iAnim < AnimCount2; iAnim++)
        {
            rANCS.ReadString();
            rANCS.Seek(0x18, SEEK_CUR);
        }

        const auto EffectGroupCount = rANCS.ReadU32();
        for (uint32_t iGrp = 0; iGrp < EffectGroupCount; iGrp++)
        {
            rANCS.ReadString();
            const auto EffectCount = rANCS.ReadU32();

            for (uint32_t iEffect = 0; iEffect < EffectCount; iEffect++)
            {
                rANCS.ReadString();
                rANCS.Seek(0x4, SEEK_CUR);
                const CAssetID ParticleID(rANCS, EIDLength::k32Bit);
                if (ParticleID.IsValid())
                    pChar->EffectParticles.push_back(ParticleID);

                if (Loader.mGame == EGame::Prime)
                    rANCS.ReadString();
                if (Loader.mGame == EGame::Echoes)
                    rANCS.Seek(0x4, SEEK_CUR);

                rANCS.Seek(0xC, SEEK_CUR);
            }
        }

        pChar->OverlayModels.push_back({
            .Type = EOverlayType::Frozen,
            .ModelID = CAssetID(rANCS, EIDLength::k32Bit),
            .SkinID = CAssetID(rANCS, EIDLength::k32Bit),
        });

        const auto AnimIndexCount = rANCS.ReadU32();

        for (uint32_t iAnim = 0; iAnim < AnimIndexCount; iAnim++)
        {
            const auto AnimIndex = rANCS.ReadU32();
            pChar->UsedAnimationIndices.insert(AnimIndex);
        }

        if (Loader.mGame == EGame::Echoes)
        {
            pChar->SpatialPrimitives = rANCS.ReadU32();
            rANCS.Seek(0x1, SEEK_CUR);
            const auto UnknownCount2 = rANCS.ReadU32();
            rANCS.Seek(UnknownCount2 * 0x1C, SEEK_CUR);
        }
    }

    // Load Animation Set
    Loader.LoadAnimationSet(rANCS);
    Loader.ProcessPrimitives();

    return ptr;
}

std::unique_ptr<CAnimSet> CAnimSetLoader::LoadCHAR(IInputStream& rCHAR, CResourceEntry *pEntry)
{
    if (!rCHAR.IsValid())
        return nullptr;

    CAnimSetLoader Loader;
    const auto Check = rCHAR.ReadU8();

    if (Check == 0x5 || Check == 0x3)
    {
        auto ptr = std::make_unique<CAnimSet>(pEntry);

        Loader.mGame = EGame::Corruption;
        Loader.pSet = ptr.get();
        Loader.LoadCorruptionCHAR(rCHAR);

        return ptr;
    }

    if (Check == 0x59)
    {
        auto ptr = std::make_unique<CAnimSet>(pEntry);

        Loader.mGame = EGame::DKCReturns;
        Loader.pSet = ptr.get();
        Loader.LoadReturnsCHAR(rCHAR);

        return ptr;
    }

    NLog::Error("{}: CHAR has invalid first byte: 0x{:02X}", rCHAR.GetSourceString(), Check);
    return nullptr;
}

std::unique_ptr<CSourceAnimData> CAnimSetLoader::LoadSAND(IInputStream& rSAND, CResourceEntry *pEntry)
{
    if (!rSAND.IsValid())
        return nullptr;

    auto* const resourceStore = pEntry->ResourceStore();

    // We only care about the transitions right now
    auto pData = std::make_unique<CSourceAnimData>(pEntry);

    [[maybe_unused]] const auto Unknown = rSAND.ReadU16(); // probably version
    ASSERT(Unknown == 0);

    // Transitions
    const auto NumTransitions = rSAND.ReadU32();

    pData->mTransitions.reserve(NumTransitions);
    for (uint32_t i = 0; i < NumTransitions; i++)
    {
        [[maybe_unused]] const auto UnkByte = rSAND.ReadU8();
        ASSERT(UnkByte == 0);

        pData->mTransitions.push_back({
            .AnimA = CAssetID(rSAND, EIDLength::k64Bit),
            .AnimB = CAssetID(rSAND, EIDLength::k64Bit),
            .pTransition = CMetaTransFactory::LoadFromStream(resourceStore, rSAND, pEntry->Game()),
        });
    }

    // Half Transitions
    const auto NumHalfTransitions = rSAND.ReadU32();

    pData->mHalfTransitions.reserve(NumHalfTransitions);
    for (uint32_t i = 0; i < NumHalfTransitions; i++)
    {
        [[maybe_unused]] const auto UnkByte = rSAND.ReadU8();
        ASSERT(UnkByte == 0);

        pData->mHalfTransitions.push_back({
            .Anim = CAssetID(rSAND, EIDLength::k64Bit),
            .pTransition = CMetaTransFactory::LoadFromStream(resourceStore, rSAND, pEntry->Game()),
        });
    }

    // Default Transition
    pData->mpDefaultTransition = CMetaTransFactory::LoadFromStream(resourceStore, rSAND, pEntry->Game());

    return pData;
}
