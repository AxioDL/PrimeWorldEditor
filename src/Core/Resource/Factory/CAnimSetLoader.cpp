#include "CAnimSetLoader.h"
#include "CAnimEventLoader.h"
#include "Core/GameProject/CResourceStore.h"
#include <Common/Log.h>

CAnimSetLoader::CAnimSetLoader()
{
}

CAnimSet* CAnimSetLoader::LoadCorruptionCHAR(IInputStream& rCHAR)
{
    pSet->mCharacters.emplace_back( SSetCharacter() );;
    SSetCharacter& rChar = pSet->mCharacters.back();

    // Character Header
    rChar.ID = rCHAR.ReadByte();
    rChar.Name = rCHAR.ReadString();
    rChar.pModel = gpResourceStore->LoadResource<CModel>(rCHAR.ReadLongLong());
    rChar.pSkin = gpResourceStore->LoadResource<CSkin>(rCHAR.ReadLongLong());

    uint32 NumOverlays = rCHAR.ReadLong();

    for (uint32 iOverlay = 0; iOverlay < NumOverlays; iOverlay++)
    {
        SOverlayModel Overlay;
        Overlay.Type = (EOverlayType) rCHAR.ReadLong();
        Overlay.ModelID = CAssetID(rCHAR, k64Bit);
        Overlay.SkinID = CAssetID(rCHAR, k64Bit);
        rChar.OverlayModels.push_back(Overlay);
    }

    rChar.pSkeleton = gpResourceStore->LoadResource<CSkeleton>(rCHAR.ReadLongLong());
    rChar.AnimDataID = CAssetID(rCHAR, k64Bit);

    // PAS Database
    LoadPASDatabase(rCHAR);

    // Particle Resource Data
    LoadParticleResourceData(rCHAR, &rChar, 10);

    // Events
    uint32 NumEventSets = rCHAR.ReadLong();

    for (uint32 iSet = 0; iSet < NumEventSets; iSet++)
    {
        CAnimEventData *pEvents = CAnimEventLoader::LoadCorruptionCharacterEventSet(rCHAR);
        pSet->mAnimEvents.push_back(pEvents);
    }

    // Animations
    uint32 NumAnimations = rCHAR.ReadLong();

    for (uint32 AnimIdx = 0; AnimIdx < NumAnimations; AnimIdx++)
    {
        SAnimation Anim;
        Anim.Name = rCHAR.ReadString();
        Anim.pMetaAnim = gMetaAnimFactory.LoadFromStream(rCHAR, mGame);
        pSet->mAnimations.push_back(Anim);
    }

    // Animation Bounds
    uint32 NumAnimationBounds = rCHAR.ReadLong();
    rCHAR.Skip(NumAnimationBounds * 0x20);
    rCHAR.Skip(1);

    // Bool Array
    uint32 BoolArraySize = rCHAR.ReadLong();
    rCHAR.Skip(BoolArraySize);

    // Collision Primitives
    uint32 NumPrimitiveSets = rCHAR.ReadLong();

    for (uint32 SetIdx = 0; SetIdx < NumPrimitiveSets; SetIdx++)
    {
        rCHAR.ReadString();
        uint32 NumPrimitives = rCHAR.ReadLong();

        for (uint32 PrimIdx = 0; PrimIdx < NumPrimitives; PrimIdx++)
        {
            rCHAR.Skip(0x34);
            rCHAR.ReadString();
            rCHAR.Skip(4);
        }
    }

    // Sound Resources
    uint32 NumSounds = rCHAR.ReadLong();

    for (uint32 SoundIdx = 0; SoundIdx < NumSounds; SoundIdx++)
    {
        CAssetID SoundID(rCHAR, k64Bit);
        rChar.SoundEffects.push_back(SoundID);
    }

    ProcessPrimitives();
    return pSet;
}

CAnimSet* CAnimSetLoader::LoadReturnsCHAR(IInputStream& rCHAR)
{
    rCHAR.Skip(0x14);
    uint8 Flag = rCHAR.ReadByte();
    rCHAR.Skip(1);

    pSet->mCharacters.emplace_back( SSetCharacter() );;
    SSetCharacter& rChar = pSet->mCharacters.back();

    // Character Header
    rChar.ID = 0;
    rChar.Name = rCHAR.ReadString();
    rChar.pSkeleton = gpResourceStore->LoadResource<CSkeleton>( rCHAR.ReadLongLong() );
    rChar.CollisionPrimitivesID = rCHAR.ReadLongLong();

    uint32 NumModels = rCHAR.ReadLong();

    for (uint32 ModelIdx = 0; ModelIdx < NumModels; ModelIdx++)
    {
        rCHAR.ReadString();
        CAssetID ModelID(rCHAR, EGame::DKCReturns);
        CAssetID SkinID(rCHAR, EGame::DKCReturns);
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
    uint32 NumAnims = rCHAR.ReadLong();

    for (uint32 AnimIdx = 0; AnimIdx < NumAnims; AnimIdx++)
    {
        TString AnimName = rCHAR.ReadString();
        CAssetID AnimID(rCHAR, EGame::DKCReturns);
        rCHAR.Skip(0x25);
        rChar.DKDependencies.push_back(AnimID);

        // small hack - create a meta-anim for it so we can generate asset names for the ANIM files correctly
        SAnimation Anim;
        Anim.Name = AnimName;
        Anim.pMetaAnim = new CMetaAnimPlay( CAnimPrimitive(AnimID, AnimIdx, AnimName), 0.f, 0 );
        pSet->mAnimations.push_back(Anim);
    }

    // The only other thing we care about right now is the dependency list. If this file doesn't have a dependency list, exit out.
    if ((Flag & 0x10) == 0)
        return pSet;

    // Anim ID Map
    if (Flag & 0x20)
    {
        uint32 NumIDs = rCHAR.ReadLong();
        rCHAR.Skip(NumIDs * 4);
    }

    // Transitions
    if (Flag & 0x80)
    {
        uint32 NumAdditiveAnims = rCHAR.ReadLong();
        rCHAR.Skip(NumAdditiveAnims * 0x10);

        uint32 NumTransitionTypes = rCHAR.ReadLong();

        for (uint32 TypeIdx = 0; TypeIdx < NumTransitionTypes; TypeIdx++)
        {
            uint16 Type = rCHAR.ReadShort();

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
                errorf("%s [0x%X]: Invalid transition type: %d", *rCHAR.GetSourceString(), rCHAR.Tell() - 2, Type);
                return pSet;
            }
        }

        uint32 NumFullTransitions = rCHAR.ReadLong();
        rCHAR.Skip(NumFullTransitions * 0xC);

        uint32 NumHalfTransitions = rCHAR.ReadLong();
        rCHAR.Skip(NumHalfTransitions * 0x8);

        rCHAR.Skip(0x8);
    }

    // Transform Bits
    if (Flag & 0x40)
    {
        uint32 NumTransformBits = rCHAR.ReadLong();
        rCHAR.Skip(NumTransformBits);
    }

    uint32 NumUnknown = rCHAR.ReadLong();
    rCHAR.Skip(NumUnknown * 4);

    // Skel Joint Sets
    uint32 NumSkelJointSets = rCHAR.ReadLong();

    for (uint32 SetIdx = 0; SetIdx < NumSkelJointSets; SetIdx++)
    {
        rCHAR.Skip(4);
        uint32 NumUnknown2 = rCHAR.ReadLong();
        rCHAR.Skip(0x20 + NumUnknown2);
    }

    // Resources
    if (Flag & 0x10)
    {
        // Don't need the extensions
        uint32 NumExtensions = rCHAR.ReadLong();
        rCHAR.Skip(NumExtensions * 4);

        uint32 NumResources = rCHAR.ReadLong();

        for (uint32 ResIdx = 0; ResIdx < NumResources; ResIdx++)
        {
            CAssetID ResID(rCHAR, EGame::DKCReturns);
            rCHAR.Skip(3);
            rChar.DKDependencies.push_back(ResID);
        }
    }

    ProcessPrimitives();
    return pSet;
}

void CAnimSetLoader::LoadPASDatabase(IInputStream& rPAS4)
{
    // For now, just parse the data; don't store it
    uint32 Magic = rPAS4.ReadLong();
    uint32 AnimStateCount = rPAS4.ReadLong();
    rPAS4.Seek(0x4, SEEK_CUR); // Skipping default anim state
    ASSERT(Magic == FOURCC('PAS4'));

    for (uint32 iState = 0; iState < AnimStateCount; iState++)
    {
        rPAS4.Seek(0x4, SEEK_CUR); // Skipping unknown value
        uint32 ParmInfoCount = rPAS4.ReadLong();
        uint32 AnimInfoCount = rPAS4.ReadLong();

        uint32 Skip = 0;
        for (uint32 iParm = 0; iParm < ParmInfoCount; iParm++)
        {
            uint32 Type = rPAS4.ReadLong();
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
            }
        }

        for (uint32 iInfo = 0; iInfo < AnimInfoCount; iInfo++)
            rPAS4.Seek(0x4 + Skip, SEEK_CUR);
    }
}

void CAnimSetLoader::LoadParticleResourceData(IInputStream& rFile, SSetCharacter *pChar, uint16 CharVersion)
{
    uint32 ParticleCount = rFile.ReadLong();
    pChar->GenericParticles.reserve(ParticleCount);

    for (uint32 iPart = 0; iPart < ParticleCount; iPart++)
        pChar->GenericParticles.push_back( CAssetID(rFile, mGame) );

    uint32 SwooshCount = rFile.ReadLong();
    pChar->SwooshParticles.reserve(SwooshCount);

    for (uint32 iSwoosh = 0; iSwoosh < SwooshCount; iSwoosh++)
        pChar->SwooshParticles.push_back( CAssetID(rFile, mGame) );

    if (CharVersion >= 6 && mGame <= EGame::Echoes) rFile.Seek(0x4, SEEK_CUR);

    uint32 ElectricCount = rFile.ReadLong();
    pChar->ElectricParticles.reserve(ElectricCount);

    for (uint32 iElec = 0; iElec < ElectricCount; iElec++)
        pChar->ElectricParticles.push_back( CAssetID(rFile, mGame) );

    if (mGame >= EGame::Echoes)
    {
        uint32 SpawnCount = rFile.ReadLong();
        pChar->SpawnParticles.reserve(SpawnCount);

        for (uint32 iSpawn = 0; iSpawn < SpawnCount; iSpawn++)
            pChar->SpawnParticles.push_back( CAssetID(rFile, mGame) );
    }

    rFile.Seek(0x4, SEEK_CUR);
    if (mGame >= EGame::Echoes) rFile.Seek(0x4, SEEK_CUR);
}

void CAnimSetLoader::LoadAnimationSet(IInputStream& rANCS)
{
    uint16 Version = rANCS.ReadShort();

    // Animations
    uint32 NumAnims = rANCS.ReadLong();
    pSet->mAnimations.reserve(NumAnims);

    for (uint32 iAnim = 0; iAnim < NumAnims; iAnim++)
    {
        SAnimation Anim;
        Anim.Name = rANCS.ReadString();
        Anim.pMetaAnim = gMetaAnimFactory.LoadFromStream(rANCS, mGame);
        pSet->mAnimations.push_back(Anim);
    }

    // Transitions
    uint32 NumTransitions = rANCS.ReadLong();
    pSet->mTransitions.reserve(NumTransitions);

    for (uint32 iTrans = 0; iTrans < NumTransitions; iTrans++)
    {
        STransition Trans;
        Trans.Unknown = rANCS.ReadLong();
        Trans.AnimIdA = rANCS.ReadLong();
        Trans.AnimIdB = rANCS.ReadLong();
        Trans.pMetaTrans = gMetaTransFactory.LoadFromStream(rANCS, mGame);
        pSet->mTransitions.push_back(Trans);
    }

    pSet->mpDefaultTransition = gMetaTransFactory.LoadFromStream(rANCS, mGame);

    // Additive Animations
    uint32 NumAdditive = rANCS.ReadLong();
    pSet->mAdditiveAnims.reserve(NumAdditive);

    for (uint32 iAdd = 0; iAdd < NumAdditive; iAdd++)
    {
        SAdditiveAnim Anim;
        Anim.AnimID = rANCS.ReadLong();
        Anim.FadeInTime = rANCS.ReadFloat();
        Anim.FadeOutTime = rANCS.ReadFloat();
        pSet->mAdditiveAnims.push_back(Anim);
    }

    pSet->mDefaultAdditiveFadeIn = rANCS.ReadFloat();
    pSet->mDefaultAdditiveFadeOut = rANCS.ReadFloat();

    // Half Transitions
    if (Version > 2)
    {
        uint32 NumHalfTransitions = rANCS.ReadLong();
        pSet->mHalfTransitions.reserve(NumHalfTransitions);

        for (uint32 iTrans = 0; iTrans < NumHalfTransitions; iTrans++)
        {
            SHalfTransition Trans;
            Trans.AnimID = rANCS.ReadLong();
            Trans.pMetaTrans = gMetaTransFactory.LoadFromStream(rANCS, mGame);
            pSet->mHalfTransitions.push_back(Trans);
        }
    }

    // Skipping MP1 ANIM asset list
    // Events
    if (mGame >= EGame::EchoesDemo)
    {
        uint32 EventDataCount = rANCS.ReadLong();
        pSet->mAnimEvents.reserve(EventDataCount);
        ASSERT(EventDataCount == NumAnims);

        for (uint32 iEvent = 0; iEvent < EventDataCount; iEvent++)
        {
            CAnimEventData *pData = CAnimEventLoader::LoadAnimSetEvents(rANCS);
            pSet->mAnimEvents.push_back(pData);
        }
    }
}

void CAnimSetLoader::ProcessPrimitives()
{
    // Find all unique anim primitives
    std::set<CAnimPrimitive> UniquePrimitives;

    for (uint32 iAnim = 0; iAnim < pSet->mAnimations.size(); iAnim++)
        pSet->mAnimations[iAnim].pMetaAnim->GetUniquePrimitives(UniquePrimitives);

    for (uint32 iTrans = 0; iTrans < pSet->mTransitions.size(); iTrans++)
        pSet->mTransitions[iTrans].pMetaTrans->GetUniquePrimitives(UniquePrimitives);

    if (pSet->mpDefaultTransition)
        pSet->mpDefaultTransition->GetUniquePrimitives(UniquePrimitives);

    for (uint32 iTrans = 0; iTrans < pSet->mHalfTransitions.size(); iTrans++)
        pSet->mHalfTransitions[iTrans].pMetaTrans->GetUniquePrimitives(UniquePrimitives);

    if (mGame == EGame::CorruptionProto || mGame == EGame::Corruption)
    {
        CSourceAnimData *pAnimData = gpResourceStore->LoadResource<CSourceAnimData>( pSet->mCharacters[0].AnimDataID );

        if (pAnimData)
            pAnimData->GetUniquePrimitives(UniquePrimitives);
    }

    // Copy anim primitives into the animset
    for (auto Iter = UniquePrimitives.begin(); Iter != UniquePrimitives.end(); Iter++)
    {
        const CAnimPrimitive& rkPrim = *Iter;
        uint32 ID = rkPrim.ID();

        if (ID >= pSet->mAnimPrimitives.size())
            pSet->mAnimPrimitives.resize(ID + 1);

        pSet->mAnimPrimitives[ID] = rkPrim;
    }

    // Add used animation indices from the animset to the character's list
    if (mGame <= EGame::Echoes)
    {
        // Add animations referenced by default transition
        if (pSet->mpDefaultTransition)
        {
            std::set<CAnimPrimitive> DefaultTransPrimitives;
            pSet->mpDefaultTransition->GetUniquePrimitives(DefaultTransPrimitives);

            for (uint32 iChar = 0; iChar < pSet->mCharacters.size(); iChar++)
            {
                SSetCharacter& rChar = pSet->mCharacters[iChar];

                for (auto Iter = DefaultTransPrimitives.begin(); Iter != DefaultTransPrimitives.end(); Iter++)
                {
                    const CAnimPrimitive& rkPrim = *Iter;
                    rChar.UsedAnimationIndices.insert(rkPrim.ID());
                }
            }
        }

        // Add animations referenced by used transitions
        for (uint32 iChar = 0; iChar < pSet->mCharacters.size(); iChar++)
        {
            SSetCharacter& rChar = pSet->mCharacters[iChar];
            bool AddedNewAnims = true;

            // Loop this until we run out of new animations. This is in case any animations
            // referenced by any transitions are also referenced by earlier transitions.
            while (AddedNewAnims)
            {
                AddedNewAnims = false;

                for (uint32 iTrans = 0; iTrans < pSet->mTransitions.size(); iTrans++)
                {
                    STransition& rTrans = pSet->mTransitions[iTrans];

                    if ( rChar.UsedAnimationIndices.find(rTrans.AnimIdA) != rChar.UsedAnimationIndices.end() &&
                         rChar.UsedAnimationIndices.find(rTrans.AnimIdB) != rChar.UsedAnimationIndices.end() )
                    {
                        std::set<CAnimPrimitive> Primitives;
                        rTrans.pMetaTrans->GetUniquePrimitives(Primitives);

                        for (auto Iter = Primitives.begin(); Iter != Primitives.end(); Iter++)
                        {
                            const CAnimPrimitive& rkPrim = *Iter;

                            if (rChar.UsedAnimationIndices.find(rkPrim.ID()) == rChar.UsedAnimationIndices.end())
                            {
                                rChar.UsedAnimationIndices.insert(rkPrim.ID());
                                AddedNewAnims = true;
                            }
                        }
                    }
                }

                for (uint32 iHalf = 0; iHalf < pSet->mHalfTransitions.size(); iHalf++)
                {
                    SHalfTransition& rTrans = pSet->mHalfTransitions[iHalf];

                    if (rChar.UsedAnimationIndices.find(rTrans.AnimID) != rChar.UsedAnimationIndices.end())
                    {
                        std::set<CAnimPrimitive> Primitives;
                        rTrans.pMetaTrans->GetUniquePrimitives(Primitives);

                        for (auto Iter = Primitives.begin(); Iter != Primitives.end(); Iter++)
                        {
                            const CAnimPrimitive& rkPrim = *Iter;

                            if (rChar.UsedAnimationIndices.find(rkPrim.ID()) == rChar.UsedAnimationIndices.end())
                            {
                                rChar.UsedAnimationIndices.insert(rkPrim.ID());
                                AddedNewAnims = true;
                            }
                        }
                    }
                }
            }
        }
    }
}

// ************ STATIC ************
CAnimSet* CAnimSetLoader::LoadANCS(IInputStream& rANCS, CResourceEntry *pEntry)
{
    if (!rANCS.IsValid()) return nullptr;

    uint32 Magic = rANCS.ReadLong();
    if (Magic != 0x00010001)
    {
        errorf("%s: Invalid ANCS magic: 0x%08X", *rANCS.GetSourceString(), Magic);
        return nullptr;
    }

    CAnimSetLoader Loader;
    Loader.pSet = new CAnimSet(pEntry);
    Loader.mGame = pEntry->Game();

    uint32 NodeCount = rANCS.ReadLong();
    Loader.pSet->mCharacters.resize(NodeCount);

    for (uint32 iNode = 0; iNode < NodeCount; iNode++)
    {
        SSetCharacter *pChar = &Loader.pSet->mCharacters[iNode];

        pChar->ID = rANCS.ReadLong();
        uint16 CharVersion = rANCS.ReadShort();
        if (iNode == 0 && Loader.mGame == EGame::Invalid)
        {
            Loader.mGame = (CharVersion == 0xA) ? EGame::Echoes : EGame::Prime;
        }
        pChar->Name = rANCS.ReadString();
        pChar->pModel = gpResourceStore->LoadResource<CModel>(rANCS.ReadLong());
        pChar->pSkin = gpResourceStore->LoadResource<CSkin>(rANCS.ReadLong());
        pChar->pSkeleton = gpResourceStore->LoadResource<CSkeleton>(rANCS.ReadLong());
        if (pChar->pModel) pChar->pModel->SetSkin(pChar->pSkin);

        // Unfortunately that's all that's actually supported at the moment. Hope to expand later.
        // Since there's no size value I have to actually read the rest of the node to reach the next one
        uint32 AnimCount = rANCS.ReadLong();

        for (uint32 iAnim = 0; iAnim < AnimCount; iAnim++)
        {
            rANCS.Seek(0x4, SEEK_CUR);
            if (Loader.mGame == EGame::Prime) rANCS.Seek(0x1, SEEK_CUR);
            rANCS.ReadString();
        }

        // PAS Database
        Loader.LoadPASDatabase(rANCS);

        // Particles
        Loader.LoadParticleResourceData(rANCS, pChar, CharVersion);

        uint32 AnimCount2 = rANCS.ReadLong();
        for (uint32 iAnim = 0; iAnim < AnimCount2; iAnim++)
        {
            rANCS.ReadString();
            rANCS.Seek(0x18, SEEK_CUR);
        }

        uint32 EffectGroupCount = rANCS.ReadLong();
        for (uint32 iGrp = 0; iGrp < EffectGroupCount; iGrp++)
        {
            rANCS.ReadString();
            uint32 EffectCount = rANCS.ReadLong();

            for (uint32 iEffect = 0; iEffect < EffectCount; iEffect++)
            {
                rANCS.ReadString();
                rANCS.Seek(0x4, SEEK_CUR);
                CAssetID ParticleID(rANCS, k32Bit);
                if (ParticleID.IsValid()) pChar->EffectParticles.push_back(ParticleID);

                if (Loader.mGame == EGame::Prime) rANCS.ReadString();
                if (Loader.mGame == EGame::Echoes) rANCS.Seek(0x4, SEEK_CUR);
                rANCS.Seek(0xC, SEEK_CUR);
            }
        }

        SOverlayModel Overlay;
        Overlay.Type = EOverlayType::Frozen;
        Overlay.ModelID = CAssetID(rANCS, k32Bit);
        Overlay.SkinID = CAssetID(rANCS, k32Bit);
        pChar->OverlayModels.push_back(Overlay);

        uint32 AnimIndexCount = rANCS.ReadLong();

        for (uint32 iAnim = 0; iAnim < AnimIndexCount; iAnim++)
        {
            uint32 AnimIndex = rANCS.ReadLong();
            pChar->UsedAnimationIndices.insert(AnimIndex);
        }

        if (Loader.mGame == EGame::Echoes)
        {
            pChar->SpatialPrimitives = rANCS.ReadLong();
            rANCS.Seek(0x1, SEEK_CUR);
            uint32 UnknownCount2 = rANCS.ReadLong();
            rANCS.Seek(UnknownCount2 * 0x1C, SEEK_CUR);
        }
    }

    // Load Animation Set
    Loader.LoadAnimationSet(rANCS);
    Loader.ProcessPrimitives();

    return Loader.pSet;
}

CAnimSet* CAnimSetLoader::LoadCHAR(IInputStream& rCHAR, CResourceEntry *pEntry)
{
    if (!rCHAR.IsValid()) return nullptr;

    CAnimSetLoader Loader;
    uint8 Check = rCHAR.ReadByte();

    if (Check == 0x5 || Check == 0x3)
    {
        Loader.mGame = EGame::Corruption;
        Loader.pSet = new CAnimSet(pEntry);
        return Loader.LoadCorruptionCHAR(rCHAR);
    }

    if (Check == 0x59)
    {
        Loader.mGame = EGame::DKCReturns;
        Loader.pSet = new CAnimSet(pEntry);
        return Loader.LoadReturnsCHAR(rCHAR);
    }

    errorf("%s: CHAR has invalid first byte: 0x%02X", *rCHAR.GetSourceString(), Check);
    return nullptr;
}

CSourceAnimData* CAnimSetLoader::LoadSAND(IInputStream& rSAND, CResourceEntry *pEntry)
{
    if (!rSAND.IsValid()) return nullptr;

    // We only care about the transitions right now
    CSourceAnimData *pData = new CSourceAnimData(pEntry);

    uint16 Unknown = rSAND.ReadShort(); // probably version
    ASSERT(Unknown == 0);

    // Transitions
    uint32 NumTransitions = rSAND.ReadLong();

    for (uint32 TransitionIdx = 0; TransitionIdx < NumTransitions; TransitionIdx++)
    {
        uint8 UnkByte = rSAND.ReadByte();
        ASSERT(UnkByte == 0);

        CSourceAnimData::STransition Transition;
        Transition.AnimA = CAssetID(rSAND, k64Bit);
        Transition.AnimB = CAssetID(rSAND, k64Bit);
        Transition.pTransition = gMetaTransFactory.LoadFromStream(rSAND, pEntry->Game());
        pData->mTransitions.push_back(Transition);
    }

    // Half Transitions
    uint32 NumHalfTransitions = rSAND.ReadLong();

    for (uint32 HalfIdx = 0; HalfIdx < NumHalfTransitions; HalfIdx++)
    {
        uint8 UnkByte = rSAND.ReadByte();
        ASSERT(UnkByte == 0);

        CSourceAnimData::SHalfTransition HalfTrans;
        HalfTrans.Anim = CAssetID(rSAND, k64Bit);
        HalfTrans.pTransition = gMetaTransFactory.LoadFromStream(rSAND, pEntry->Game());
        pData->mHalfTransitions.push_back(HalfTrans);
    }

    // Default Transition
    pData->mpDefaultTransition = gMetaTransFactory.LoadFromStream(rSAND, pEntry->Game());

    return pData;
}
