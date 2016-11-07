#include "CAnimSetLoader.h"
#include "CAnimEventLoader.h"
#include "Core/GameProject/CResourceStore.h"
#include <Common/Log.h>

CAnimSetLoader::CAnimSetLoader()
{
}

CAnimSet* CAnimSetLoader::LoadCorruptionCHAR(IInputStream& rCHAR)
{
    // For now, we only read enough to fetch the model
    rCHAR.Seek(0x1, SEEK_CUR);
    pSet->mCharacters.resize(1);
    SSetCharacter& rNode = pSet->mCharacters[0];

    rNode.Name = rCHAR.ReadString();
    rNode.pModel = gpResourceStore->LoadResource(rCHAR.ReadLongLong(), "CMDL");
    return pSet;
}

CAnimSet* CAnimSetLoader::LoadReturnsCHAR(IInputStream& rCHAR)
{
    // For now, we only read enough to fetch the model
    rCHAR.Seek(0x16, SEEK_CUR);
    pSet->mCharacters.resize(1);
    SSetCharacter& rNode = pSet->mCharacters[0];

    rNode.Name = rCHAR.ReadString();
    rCHAR.Seek(0x14, SEEK_CUR);
    rCHAR.ReadString();
    rNode.pModel = gpResourceStore->LoadResource(rCHAR.ReadLongLong(), "CMDL");
    return pSet;
}

void CAnimSetLoader::LoadPASDatabase(IInputStream& rPAS4)
{
    // For now, just parse the data; don't store it
    rPAS4.Seek(0x4, SEEK_CUR); // Skipping PAS4 FourCC
    u32 AnimStateCount = rPAS4.ReadLong();
    rPAS4.Seek(0x4, SEEK_CUR); // Skipping default anim state

    for (u32 iState = 0; iState < AnimStateCount; iState++)
    {
        rPAS4.Seek(0x4, SEEK_CUR); // Skipping unknown value
        u32 ParmInfoCount = rPAS4.ReadLong();
        u32 AnimInfoCount = rPAS4.ReadLong();

        u32 Skip = 0;
        for (u32 iParm = 0; iParm < ParmInfoCount; iParm++)
        {
            u32 Type = rPAS4.ReadLong();
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

        for (u32 iInfo = 0; iInfo < AnimInfoCount; iInfo++)
            rPAS4.Seek(0x4 + Skip, SEEK_CUR);
    }
}

void CAnimSetLoader::LoadAnimationSet(IInputStream& rANCS)
{
    u16 Version = rANCS.ReadShort();

    // Animations
    u32 NumAnims = rANCS.ReadLong();
    pSet->mAnimations.reserve(NumAnims);

    for (u32 iAnim = 0; iAnim < NumAnims; iAnim++)
    {
        SAnimation Anim;
        Anim.Name = rANCS.ReadString();
        Anim.pMetaAnim = gMetaAnimFactory.LoadFromStream(rANCS);
        pSet->mAnimations.push_back(Anim);
    }

    // Transitions
    u32 NumTransitions = rANCS.ReadLong();
    pSet->mTransitions.reserve(NumTransitions);

    for (u32 iTrans = 0; iTrans < NumTransitions; iTrans++)
    {
        STransition Trans;
        Trans.Unknown = rANCS.ReadLong();
        Trans.AnimIdA = rANCS.ReadLong();
        Trans.AnimIdB = rANCS.ReadLong();
        Trans.pMetaTrans = gMetaTransFactory.LoadFromStream(rANCS);
        pSet->mTransitions.push_back(Trans);
    }

    pSet->mpDefaultTransition = gMetaTransFactory.LoadFromStream(rANCS);

    // Additive Animations
    u32 NumAdditive = rANCS.ReadLong();
    pSet->mAdditiveAnims.reserve(NumAdditive);

    for (u32 iAdd = 0; iAdd < NumAdditive; iAdd++)
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
        u32 NumHalfTransitions = rANCS.ReadLong();
        pSet->mHalfTransitions.reserve(NumHalfTransitions);

        for (u32 iTrans = 0; iTrans < NumHalfTransitions; iTrans++)
        {
            SHalfTransition Trans;
            Trans.AnimID = rANCS.ReadLong();
            Trans.pMetaTrans = gMetaTransFactory.LoadFromStream(rANCS);
            pSet->mHalfTransitions.push_back(Trans);
        }
    }

    // Skipping MP1 ANIM asset list
    // Events
    if (mVersion >= eEchoesDemo)
    {
        u32 EventDataCount = rANCS.ReadLong();
        pSet->mAnimEvents.reserve(EventDataCount);
        ASSERT(EventDataCount == NumAnims);

        for (u32 iEvent = 0; iEvent < EventDataCount; iEvent++)
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

    for (u32 iAnim = 0; iAnim < pSet->mAnimations.size(); iAnim++)
        pSet->mAnimations[iAnim].pMetaAnim->GetUniquePrimitives(UniquePrimitives);

    for (u32 iTrans = 0; iTrans < pSet->mTransitions.size(); iTrans++)
        pSet->mTransitions[iTrans].pMetaTrans->GetUniquePrimitives(UniquePrimitives);

    pSet->mpDefaultTransition->GetUniquePrimitives(UniquePrimitives);

    for (u32 iTrans = 0; iTrans < pSet->mHalfTransitions.size(); iTrans++)
        pSet->mHalfTransitions[iTrans].pMetaTrans->GetUniquePrimitives(UniquePrimitives);

    // Copy anim primitives into the animset
    for (auto Iter = UniquePrimitives.begin(); Iter != UniquePrimitives.end(); Iter++)
    {
        const CAnimPrimitive& rkPrim = *Iter;
        u32 ID = rkPrim.ID();

        if (ID >= pSet->mAnimPrimitives.size())
            pSet->mAnimPrimitives.resize(ID + 1);

        pSet->mAnimPrimitives[ID] = rkPrim;
        ASSERT(pSet->Animation(ID)->pMetaAnim->Type() == eMAT_Play);
    }

    // Add animations referenced by default transition
    std::set<CAnimPrimitive> DefaultTransPrimitives;
    pSet->mpDefaultTransition->GetUniquePrimitives(DefaultTransPrimitives);

    for (u32 iChar = 0; iChar < pSet->mCharacters.size(); iChar++)
    {
        SSetCharacter& rChar = pSet->mCharacters[iChar];

        for (auto Iter = DefaultTransPrimitives.begin(); Iter != DefaultTransPrimitives.end(); Iter++)
        {
            const CAnimPrimitive& rkPrim = *Iter;
            rChar.UsedAnimationIndices.insert(rkPrim.ID());
        }
    }

    // Add animations referenced by used transitions
    for (u32 iChar = 0; iChar < pSet->mCharacters.size(); iChar++)
    {
        SSetCharacter& rChar = pSet->mCharacters[iChar];
        bool AddedNewAnims = true;

        // Loop this until we run out of new animations. This is in case any animations
        // referenced by any transitions are also referenced by earlier transitions.
        while (AddedNewAnims)
        {
            AddedNewAnims = false;

            for (u32 iTrans = 0; iTrans < pSet->mTransitions.size(); iTrans++)
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

            for (u32 iHalf = 0; iHalf < pSet->mHalfTransitions.size(); iHalf++)
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

// ************ STATIC ************
CAnimSet* CAnimSetLoader::LoadANCSOrCHAR(IInputStream& rFile, CResourceEntry *pEntry)
{
    if (!rFile.IsValid()) return nullptr;
    u8 Test = rFile.PeekByte();

    if (Test == 0x3 || Test == 0x5 || Test == 0x59)
        return LoadCHAR(rFile, pEntry);
    else if (Test == 0x0)
        return LoadANCS(rFile, pEntry);

    Log::Error("Failed to determine animset format for " + rFile.GetSourceString() + "; first byte is " + TString::HexString(Test, 2));
    return nullptr;
}

CAnimSet* CAnimSetLoader::LoadANCS(IInputStream& rANCS, CResourceEntry *pEntry)
{
    if (!rANCS.IsValid()) return nullptr;

    u32 Magic = rANCS.ReadLong();
    if (Magic != 0x00010001)
    {
        Log::FileError(rANCS.GetSourceString(), "Invalid ANCS magic: " + TString::HexString(Magic));
        return nullptr;
    }

    CAnimSetLoader Loader;
    Loader.pSet = new CAnimSet(pEntry);
    Loader.mVersion = pEntry->Game();

    u32 NodeCount = rANCS.ReadLong();
    Loader.pSet->mCharacters.resize(NodeCount);

    for (u32 iNode = 0; iNode < NodeCount; iNode++)
    {
        SSetCharacter *pChar = &Loader.pSet->mCharacters[iNode];

        rANCS.Seek(0x4, SEEK_CUR); // Skipping node self-index
        u16 Unknown1 = rANCS.ReadShort();
        if (iNode == 0 && Loader.mVersion == eUnknownGame)
        {
            Loader.mVersion = (Unknown1 == 0xA) ? eEchoes : ePrime; // Best version indicator we know of unfortunately
            Loader.pSet->SetGame(Loader.mVersion);
        }
        pChar->Name = rANCS.ReadString();
        pChar->pModel = gpResourceStore->LoadResource(rANCS.ReadLong(), "CMDL");
        pChar->pSkin = gpResourceStore->LoadResource(rANCS.ReadLong(), "CSKR");
        pChar->pSkeleton = gpResourceStore->LoadResource(rANCS.ReadLong(), "CINF");
        if (pChar->pModel) pChar->pModel->SetSkin(pChar->pSkin);

        // Unfortunately that's all that's actually supported at the moment. Hope to expand later.
        // Since there's no size value I have to actually read the rest of the node to reach the next one
        u32 AnimCount = rANCS.ReadLong();

        for (u32 iAnim = 0; iAnim < AnimCount; iAnim++)
        {
            rANCS.Seek(0x4, SEEK_CUR);
            if (Loader.mVersion == ePrime) rANCS.Seek(0x1, SEEK_CUR);
            rANCS.ReadString();
        }

        // PAS Database
        Loader.LoadPASDatabase(rANCS);

        // Particles
        u32 ParticleCount = rANCS.ReadLong();
        pChar->GenericParticles.reserve(ParticleCount);

        for (u32 iPart = 0; iPart < ParticleCount; iPart++)
            pChar->GenericParticles.push_back( CAssetID(rANCS, e32Bit) );

        u32 SwooshCount = rANCS.ReadLong();
        pChar->SwooshParticles.reserve(SwooshCount);

        for (u32 iSwoosh = 0; iSwoosh < SwooshCount; iSwoosh++)
            pChar->SwooshParticles.push_back( CAssetID(rANCS, e32Bit) );

        if (Unknown1 != 5) rANCS.Seek(0x4, SEEK_CUR);

        u32 ElectricCount = rANCS.ReadLong();
        pChar->ElectricParticles.reserve(ElectricCount);

        for (u32 iElec = 0; iElec < ElectricCount; iElec++)
            pChar->ElectricParticles.push_back( CAssetID(rANCS, e32Bit) );

        if (Loader.mVersion == eEchoes)
        {
            u32 SpawnCount = rANCS.ReadLong();
            pChar->SpawnParticles.reserve(SpawnCount);

            for (u32 iSpawn = 0; iSpawn < SpawnCount; iSpawn++)
                pChar->SpawnParticles.push_back( CAssetID(rANCS, e32Bit) );
        }

        rANCS.Seek(0x4, SEEK_CUR);
        if (Loader.mVersion == eEchoes) rANCS.Seek(0x4, SEEK_CUR);

        u32 AnimCount2 = rANCS.ReadLong();
        for (u32 iAnim = 0; iAnim < AnimCount2; iAnim++)
        {
            rANCS.ReadString();
            rANCS.Seek(0x18, SEEK_CUR);
        }

        u32 EffectGroupCount = rANCS.ReadLong();
        for (u32 iGrp = 0; iGrp < EffectGroupCount; iGrp++)
        {
            rANCS.ReadString();
            u32 EffectCount = rANCS.ReadLong();

            for (u32 iEffect = 0; iEffect < EffectCount; iEffect++)
            {
                rANCS.ReadString();
                rANCS.Seek(0x4, SEEK_CUR);
                CAssetID ParticleID(rANCS, e32Bit);
                if (ParticleID.IsValid()) pChar->EffectParticles.push_back(ParticleID);

                if (Loader.mVersion == ePrime) rANCS.ReadString();
                if (Loader.mVersion == eEchoes) rANCS.Seek(0x4, SEEK_CUR);
                rANCS.Seek(0xC, SEEK_CUR);
            }
        }
        pChar->IceModel = CAssetID(rANCS, e32Bit);
        pChar->IceSkin = CAssetID(rANCS, e32Bit);

        u32 AnimIndexCount = rANCS.ReadLong();

        for (u32 iAnim = 0; iAnim < AnimIndexCount; iAnim++)
        {
            u32 AnimIndex = rANCS.ReadLong();
            pChar->UsedAnimationIndices.insert(AnimIndex);
        }

        if (Loader.mVersion == eEchoes)
        {
            pChar->SpatialPrimitives = rANCS.ReadLong();
            rANCS.Seek(0x1, SEEK_CUR);
            u32 UnknownCount2 = rANCS.ReadLong();
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
    u8 Check = rCHAR.ReadByte();

    if (Check == 0x5 || Check == 0x3)
    {
        Loader.mVersion = eCorruption;
        Loader.pSet = new CAnimSet(pEntry);
        Loader.pSet->SetGame(eCorruption);
        return Loader.LoadCorruptionCHAR(rCHAR);
    }

    if (Check == 0x59)
    {
        Loader.mVersion = eReturns;
        Loader.pSet = new CAnimSet(pEntry);
        Loader.pSet->SetGame(eReturns);
        return Loader.LoadReturnsCHAR(rCHAR);
    }

    Log::FileError(rCHAR.GetSourceString(), "CHAR has invalid first byte: " + TString::HexString(Check, 2));
    return nullptr;
}
