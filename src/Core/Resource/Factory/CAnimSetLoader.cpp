#include "CAnimSetLoader.h"
#include "CAnimEventLoader.h"
#include "Core/GameProject/CResourceStore.h"
#include <Common/Log.h>

CAnimSetLoader::CAnimSetLoader() = default;

void CAnimSetLoader::LoadCorruptionCHAR(IInputStream& rCHAR)
{
    SSetCharacter& rChar = pSet->mCharacters.emplace_back();

    // Character Header
    rChar.ID = rCHAR.ReadUByte();
    rChar.Name = rCHAR.ReadString();
    rChar.pModel = gpResourceStore->LoadResource<CModel>(rCHAR.ReadULongLong());
    rChar.pSkin = gpResourceStore->LoadResource<CSkin>(rCHAR.ReadULongLong());

    const uint32 NumOverlays = rCHAR.ReadULong();

    for (uint32 iOverlay = 0; iOverlay < NumOverlays; iOverlay++)
    {
        SOverlayModel Overlay;
        Overlay.Type = static_cast<EOverlayType>(rCHAR.ReadULong());
        Overlay.ModelID = CAssetID(rCHAR, EIDLength::k64Bit);
        Overlay.SkinID = CAssetID(rCHAR, EIDLength::k64Bit);
        rChar.OverlayModels.push_back(Overlay);
    }

    rChar.pSkeleton = gpResourceStore->LoadResource<CSkeleton>(rCHAR.ReadLongLong());
    rChar.AnimDataID = CAssetID(rCHAR, EIDLength::k64Bit);

    // PAS Database
    LoadPASDatabase(rCHAR);

    // Particle Resource Data
    LoadParticleResourceData(rCHAR, &rChar, 10);

    // Events
    const uint32 NumEventSets = rCHAR.ReadULong();

    for (uint32 iSet = 0; iSet < NumEventSets; iSet++)
    {
        pSet->mAnimEvents.push_back(CAnimEventLoader::LoadCorruptionCharacterEventSet(rCHAR));
    }

    // Animations
    const uint32 NumAnimations = rCHAR.ReadULong();

    for (uint32 AnimIdx = 0; AnimIdx < NumAnimations; AnimIdx++)
    {
        SAnimation Anim;
        Anim.Name = rCHAR.ReadString();
        Anim.pMetaAnim = gMetaAnimFactory.LoadFromStream(rCHAR, mGame);
        pSet->mAnimations.push_back(std::move(Anim));
    }

    // Animation Bounds
    const uint32 NumAnimationBounds = rCHAR.ReadULong();
    rCHAR.Skip(NumAnimationBounds * 0x20);
    rCHAR.Skip(1);

    // Bool Array
    const uint32 BoolArraySize = rCHAR.ReadULong();
    rCHAR.Skip(BoolArraySize);

    // Collision Primitives
    const uint32 NumPrimitiveSets = rCHAR.ReadULong();

    for (uint32 SetIdx = 0; SetIdx < NumPrimitiveSets; SetIdx++)
    {
        rCHAR.ReadString();
        const uint32 NumPrimitives = rCHAR.ReadULong();

        for (uint32 PrimIdx = 0; PrimIdx < NumPrimitives; PrimIdx++)
        {
            rCHAR.Skip(0x34);
            rCHAR.ReadString();
            rCHAR.Skip(4);
        }
    }

    // Sound Resources
    const uint32 NumSounds = rCHAR.ReadULong();

    for (uint32 SoundIdx = 0; SoundIdx < NumSounds; SoundIdx++)
    {
        const CAssetID SoundID(rCHAR, EIDLength::k64Bit);
        rChar.SoundEffects.push_back(SoundID);
    }

    ProcessPrimitives();
}

void CAnimSetLoader::LoadReturnsCHAR(IInputStream& rCHAR)
{
    rCHAR.Skip(0x14);
    const uint8 Flag = rCHAR.ReadUByte();
    rCHAR.Skip(1);

    SSetCharacter& rChar = pSet->mCharacters.emplace_back();

    // Character Header
    rChar.ID = 0;
    rChar.Name = rCHAR.ReadString();
    rChar.pSkeleton = gpResourceStore->LoadResource<CSkeleton>( rCHAR.ReadLongLong() );
    rChar.CollisionPrimitivesID = rCHAR.ReadLongLong();

    const uint32 NumModels = rCHAR.ReadULong();

    for (uint32 ModelIdx = 0; ModelIdx < NumModels; ModelIdx++)
    {
        rCHAR.ReadString();
        const CAssetID ModelID(rCHAR, EGame::DKCReturns);
        const CAssetID SkinID(rCHAR, EGame::DKCReturns);
        rCHAR.Skip(0x18);

        if (ModelIdx == 0)
        {
            rChar.pModel = gpResourceStore->LoadResource<CModel>(ModelID);
            rChar.pSkin = gpResourceStore->LoadResource<CSkin>(SkinID);
        }
        else
        {
            rChar.DKDependencies.push_back(ModelID);
            rChar.DKDependencies.push_back(SkinID);
        }
    }

    // Animations
    const uint32 NumAnims = rCHAR.ReadULong();

    for (uint32 AnimIdx = 0; AnimIdx < NumAnims; AnimIdx++)
    {
        TString AnimName = rCHAR.ReadString();
        const CAssetID AnimID(rCHAR, EGame::DKCReturns);
        rCHAR.Skip(0x25);
        rChar.DKDependencies.push_back(AnimID);

        // small hack - create a meta-anim for it so we can generate asset names for the ANIM files correctly
        SAnimation Anim;
        Anim.pMetaAnim = std::make_unique<CMetaAnimPlay>(CAnimPrimitive(AnimID, AnimIdx, AnimName), 0.f, 0);
        Anim.Name = std::move(AnimName);
        pSet->mAnimations.push_back(std::move(Anim));
    }

    // The only other thing we care about right now is the dependency list. If this file doesn't have a dependency list, exit out.
    if ((Flag & 0x10) == 0)
        return;

    // Anim ID Map
    if ((Flag & 0x20) != 0)
    {
        uint32 NumIDs = rCHAR.ReadLong();
        rCHAR.Skip(NumIDs * 4);
    }

    // Transitions
    if ((Flag & 0x80) != 0)
    {
        const uint32 NumAdditiveAnims = rCHAR.ReadULong();
        rCHAR.Skip(NumAdditiveAnims * 0x10);

        const uint32 NumTransitionTypes = rCHAR.ReadULong();

        for (uint32 TypeIdx = 0; TypeIdx < NumTransitionTypes; TypeIdx++)
        {
            const uint16 Type = rCHAR.ReadUShort();

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
                errorf("%s [0x%X]: Invalid transition type: %u", *rCHAR.GetSourceString(), rCHAR.Tell() - 2, Type);
                return;
            }
        }

        const uint32 NumFullTransitions = rCHAR.ReadULong();
        rCHAR.Skip(NumFullTransitions * 0xC);

        const uint32 NumHalfTransitions = rCHAR.ReadULong();
        rCHAR.Skip(NumHalfTransitions * 0x8);

        rCHAR.Skip(0x8);
    }

    // Transform Bits
    if ((Flag & 0x40) != 0)
    {
        const uint32 NumTransformBits = rCHAR.ReadULong();
        rCHAR.Skip(NumTransformBits);
    }

    const uint32 NumUnknown = rCHAR.ReadULong();
    rCHAR.Skip(NumUnknown * 4);

    // Skel Joint Sets
    const uint32 NumSkelJointSets = rCHAR.ReadULong();

    for (uint32 SetIdx = 0; SetIdx < NumSkelJointSets; SetIdx++)
    {
        rCHAR.Skip(4);
        const uint32 NumUnknown2 = rCHAR.ReadULong();
        rCHAR.Skip(0x20 + NumUnknown2);
    }

    // Resources
    if ((Flag & 0x10) != 0)
    {
        // Don't need the extensions
        const uint32 NumExtensions = rCHAR.ReadULong();
        rCHAR.Skip(NumExtensions * 4);

        const uint32 NumResources = rCHAR.ReadULong();

        for (uint32 ResIdx = 0; ResIdx < NumResources; ResIdx++)
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
    [[maybe_unused]] const uint32 Magic = rPAS4.ReadULong();
    const uint32 AnimStateCount = rPAS4.ReadULong();
    rPAS4.Seek(0x4, SEEK_CUR); // Skipping default anim state
    ASSERT(Magic == FOURCC('PAS4'));

    for (uint32 iState = 0; iState < AnimStateCount; iState++)
    {
        rPAS4.Seek(0x4, SEEK_CUR); // Skipping unknown value
        const uint32 ParmInfoCount = rPAS4.ReadULong();
        const uint32 AnimInfoCount = rPAS4.ReadULong();

        uint32 Skip = 0;
        for (uint32 iParm = 0; iParm < ParmInfoCount; iParm++)
        {
            const uint32 Type = rPAS4.ReadULong();
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

        for (uint32 iInfo = 0; iInfo < AnimInfoCount; iInfo++)
            rPAS4.Seek(0x4 + Skip, SEEK_CUR);
    }
}

void CAnimSetLoader::LoadParticleResourceData(IInputStream& rFile, SSetCharacter *pChar, uint16 CharVersion)
{
    const uint32 ParticleCount = rFile.ReadULong();
    pChar->GenericParticles.reserve(ParticleCount);

    for (uint32 iPart = 0; iPart < ParticleCount; iPart++)
        pChar->GenericParticles.emplace_back(rFile, mGame);

    const uint32 SwooshCount = rFile.ReadULong();
    pChar->SwooshParticles.reserve(SwooshCount);

    for (uint32 iSwoosh = 0; iSwoosh < SwooshCount; iSwoosh++)
        pChar->SwooshParticles.emplace_back(rFile, mGame);

    if (CharVersion >= 6 && mGame <= EGame::Echoes) rFile.Seek(0x4, SEEK_CUR);

    const uint32 ElectricCount = rFile.ReadULong();
    pChar->ElectricParticles.reserve(ElectricCount);

    for (uint32 iElec = 0; iElec < ElectricCount; iElec++)
        pChar->ElectricParticles.emplace_back(rFile, mGame);

    if (mGame >= EGame::Echoes)
    {
        const uint32 SpawnCount = rFile.ReadULong();
        pChar->SpawnParticles.reserve(SpawnCount);

        for (uint32 iSpawn = 0; iSpawn < SpawnCount; iSpawn++)
            pChar->SpawnParticles.emplace_back(rFile, mGame);
    }

    rFile.Seek(0x4, SEEK_CUR);
    if (mGame >= EGame::Echoes)
        rFile.Seek(0x4, SEEK_CUR);
}

void CAnimSetLoader::LoadAnimationSet(IInputStream& rANCS)
{
    const uint16 Version = rANCS.ReadUShort();

    // Animations
    const uint32 NumAnims = rANCS.ReadULong();
    pSet->mAnimations.reserve(NumAnims);

    for (uint32 iAnim = 0; iAnim < NumAnims; iAnim++)
    {
        SAnimation Anim;
        Anim.Name = rANCS.ReadString();
        Anim.pMetaAnim = gMetaAnimFactory.LoadFromStream(rANCS, mGame);
        pSet->mAnimations.push_back(std::move(Anim));
    }

    // Transitions
    const uint32 NumTransitions = rANCS.ReadULong();
    pSet->mTransitions.reserve(NumTransitions);

    for (uint32 iTrans = 0; iTrans < NumTransitions; iTrans++)
    {
        STransition Trans;
        Trans.Unknown = rANCS.ReadULong();
        Trans.AnimIdA = rANCS.ReadULong();
        Trans.AnimIdB = rANCS.ReadULong();
        Trans.pMetaTrans = gMetaTransFactory.LoadFromStream(rANCS, mGame);
        pSet->mTransitions.push_back(std::move(Trans));
    }

    pSet->mpDefaultTransition = gMetaTransFactory.LoadFromStream(rANCS, mGame);

    // Additive Animations
    const uint32 NumAdditive = rANCS.ReadULong();
    pSet->mAdditiveAnims.reserve(NumAdditive);

    for (uint32 iAdd = 0; iAdd < NumAdditive; iAdd++)
    {
        const SAdditiveAnim Anim{
            rANCS.ReadULong(),
            rANCS.ReadFloat(),
            rANCS.ReadFloat(),
        };
        pSet->mAdditiveAnims.push_back(Anim);
    }

    pSet->mDefaultAdditiveFadeIn = rANCS.ReadFloat();
    pSet->mDefaultAdditiveFadeOut = rANCS.ReadFloat();

    // Half Transitions
    if (Version > 2)
    {
        const uint32 NumHalfTransitions = rANCS.ReadULong();
        pSet->mHalfTransitions.reserve(NumHalfTransitions);

        for (uint32 iTrans = 0; iTrans < NumHalfTransitions; iTrans++)
        {
            SHalfTransition Trans;
            Trans.AnimID = rANCS.ReadULong();
            Trans.pMetaTrans = gMetaTransFactory.LoadFromStream(rANCS, mGame);
            pSet->mHalfTransitions.push_back(std::move(Trans));
        }
    }

    // Skipping MP1 ANIM asset list
    // Events
    if (mGame >= EGame::EchoesDemo)
    {
        const uint32 EventDataCount = rANCS.ReadULong();
        pSet->mAnimEvents.reserve(EventDataCount);
        ASSERT(EventDataCount == NumAnims);

        for (uint32 iEvent = 0; iEvent < EventDataCount; iEvent++)
        {
            pSet->mAnimEvents.push_back(CAnimEventLoader::LoadAnimSetEvents(rANCS));
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
        CSourceAnimData *pAnimData = gpResourceStore->LoadResource<CSourceAnimData>( pSet->mCharacters[0].AnimDataID );

        if (pAnimData != nullptr)
            pAnimData->GetUniquePrimitives(UniquePrimitives);
    }

    // Copy anim primitives into the animset
    for (const auto& primitive : UniquePrimitives)
    {
        const uint32 ID = primitive.ID();

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
                    if (character.UsedAnimationIndices.find(transition.AnimIdA) == character.UsedAnimationIndices.cend() ||
                        character.UsedAnimationIndices.find(transition.AnimIdB) == character.UsedAnimationIndices.cend())
                    {
                        continue;
                    }

                    std::set<CAnimPrimitive> Primitives;
                    transition.pMetaTrans->GetUniquePrimitives(Primitives);

                    for (const auto& primitive : Primitives)
                    {
                        if (character.UsedAnimationIndices.find(primitive.ID()) == character.UsedAnimationIndices.cend())
                        {
                            character.UsedAnimationIndices.insert(primitive.ID());
                            AddedNewAnims = true;
                        }
                    }
                }

                for (SHalfTransition& trans : pSet->mHalfTransitions)
                {
                    if (character.UsedAnimationIndices.find(trans.AnimID) == character.UsedAnimationIndices.cend())
                        continue;

                    std::set<CAnimPrimitive> Primitives;
                    trans.pMetaTrans->GetUniquePrimitives(Primitives);

                    for (const auto& primitive : Primitives)
                    {
                        if (character.UsedAnimationIndices.find(primitive.ID()) == character.UsedAnimationIndices.cend())
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

    const uint32 Magic = rANCS.ReadULong();
    if (Magic != 0x00010001)
    {
        errorf("%s: Invalid ANCS magic: 0x%08X", *rANCS.GetSourceString(), Magic);
        return nullptr;
    }

    auto ptr = std::make_unique<CAnimSet>(pEntry);

    CAnimSetLoader Loader;
    Loader.pSet = ptr.get();
    Loader.mGame = pEntry->Game();

    const uint32 NodeCount = rANCS.ReadULong();
    Loader.pSet->mCharacters.resize(NodeCount);

    for (uint32 iNode = 0; iNode < NodeCount; iNode++)
    {
        SSetCharacter *pChar = &Loader.pSet->mCharacters[iNode];

        pChar->ID = rANCS.ReadULong();
        const uint16 CharVersion = rANCS.ReadUShort();
        if (iNode == 0 && Loader.mGame == EGame::Invalid)
        {
            Loader.mGame = (CharVersion == 0xA) ? EGame::Echoes : EGame::Prime;
        }
        pChar->Name = rANCS.ReadString();
        pChar->pModel = gpResourceStore->LoadResource<CModel>(rANCS.ReadULong());
        pChar->pSkin = gpResourceStore->LoadResource<CSkin>(rANCS.ReadULong());
        pChar->pSkeleton = gpResourceStore->LoadResource<CSkeleton>(rANCS.ReadULong());
        if (pChar->pModel != nullptr)
            pChar->pModel->SetSkin(pChar->pSkin);

        // Unfortunately that's all that's actually supported at the moment. Hope to expand later.
        // Since there's no size value I have to actually read the rest of the node to reach the next one
        const uint32 AnimCount = rANCS.ReadULong();

        for (uint32 iAnim = 0; iAnim < AnimCount; iAnim++)
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

        const uint32 AnimCount2 = rANCS.ReadULong();
        for (uint32 iAnim = 0; iAnim < AnimCount2; iAnim++)
        {
            rANCS.ReadString();
            rANCS.Seek(0x18, SEEK_CUR);
        }

        const uint32 EffectGroupCount = rANCS.ReadULong();
        for (uint32 iGrp = 0; iGrp < EffectGroupCount; iGrp++)
        {
            rANCS.ReadString();
            const uint32 EffectCount = rANCS.ReadULong();

            for (uint32 iEffect = 0; iEffect < EffectCount; iEffect++)
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

        SOverlayModel Overlay;
        Overlay.Type = EOverlayType::Frozen;
        Overlay.ModelID = CAssetID(rANCS, EIDLength::k32Bit);
        Overlay.SkinID = CAssetID(rANCS, EIDLength::k32Bit);
        pChar->OverlayModels.push_back(Overlay);

        const uint32 AnimIndexCount = rANCS.ReadULong();

        for (uint32 iAnim = 0; iAnim < AnimIndexCount; iAnim++)
        {
            const uint32 AnimIndex = rANCS.ReadULong();
            pChar->UsedAnimationIndices.insert(AnimIndex);
        }

        if (Loader.mGame == EGame::Echoes)
        {
            pChar->SpatialPrimitives = rANCS.ReadULong();
            rANCS.Seek(0x1, SEEK_CUR);
            const uint32 UnknownCount2 = rANCS.ReadULong();
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
    const uint8 Check = rCHAR.ReadUByte();

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

    errorf("%s: CHAR has invalid first byte: 0x%02X", *rCHAR.GetSourceString(), Check);
    return nullptr;
}

std::unique_ptr<CSourceAnimData> CAnimSetLoader::LoadSAND(IInputStream& rSAND, CResourceEntry *pEntry)
{
    if (!rSAND.IsValid())
        return nullptr;

    // We only care about the transitions right now
    auto pData = std::make_unique<CSourceAnimData>(pEntry);

    [[maybe_unused]] const uint16 Unknown = rSAND.ReadUShort(); // probably version
    ASSERT(Unknown == 0);

    // Transitions
    const uint32 NumTransitions = rSAND.ReadULong();

    for (uint32 TransitionIdx = 0; TransitionIdx < NumTransitions; TransitionIdx++)
    {
        [[maybe_unused]] const uint8 UnkByte = rSAND.ReadUByte();
        ASSERT(UnkByte == 0);

        CSourceAnimData::STransition Transition;
        Transition.AnimA = CAssetID(rSAND, EIDLength::k64Bit);
        Transition.AnimB = CAssetID(rSAND, EIDLength::k64Bit);
        Transition.pTransition = gMetaTransFactory.LoadFromStream(rSAND, pEntry->Game());
        pData->mTransitions.push_back(std::move(Transition));
    }

    // Half Transitions
    const uint32 NumHalfTransitions = rSAND.ReadULong();

    for (uint32 HalfIdx = 0; HalfIdx < NumHalfTransitions; HalfIdx++)
    {
        [[maybe_unused]] const uint8 UnkByte = rSAND.ReadUByte();
        ASSERT(UnkByte == 0);

        CSourceAnimData::SHalfTransition HalfTrans;
        HalfTrans.Anim = CAssetID(rSAND, EIDLength::k64Bit);
        HalfTrans.pTransition = gMetaTransFactory.LoadFromStream(rSAND, pEntry->Game());
        pData->mHalfTransitions.push_back(std::move(HalfTrans));
    }

    // Default Transition
    pData->mpDefaultTransition = gMetaTransFactory.LoadFromStream(rSAND, pEntry->Game());

    return pData;
}
