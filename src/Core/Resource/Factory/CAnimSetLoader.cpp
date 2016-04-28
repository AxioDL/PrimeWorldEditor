#include "CAnimSetLoader.h"
#include "Core/Resource/CResCache.h"
#include <Common/Log.h>

CAnimSetLoader::CAnimSetLoader()
{
}

CAnimSet* CAnimSetLoader::LoadCorruptionCHAR(IInputStream& rCHAR)
{
    // For now, we only read enough to fetch the model
    rCHAR.Seek(0x1, SEEK_CUR);
    pSet->mNodes.resize(1);
    CAnimSet::SNode& node = pSet->mNodes[0];

    node.Name = rCHAR.ReadString();
    node.pModel = gResCache.GetResource(rCHAR.ReadLongLong(), "CMDL");
    return pSet;
}

CAnimSet* CAnimSetLoader::LoadReturnsCHAR(IInputStream& rCHAR)
{
    // For now, we only read enough to fetch the model
    rCHAR.Seek(0x16, SEEK_CUR);
    pSet->mNodes.resize(1);
    CAnimSet::SNode& rNode = pSet->mNodes[0];

    rNode.Name = rCHAR.ReadString();
    rCHAR.Seek(0x14, SEEK_CUR);
    rCHAR.ReadString();
    rNode.pModel = gResCache.GetResource(rCHAR.ReadLongLong(), "CMDL");
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

void CAnimSetLoader::LoadAnimation(IInputStream& rANCS)
{
    TString Name = rANCS.ReadString();
    Name = Name;
    LoadMetaAnimation(rANCS);
}

void CAnimSetLoader::LoadMetaAnimation(IInputStream& rANCS)
{
    u32 Type = rANCS.ReadLong();

    switch (Type)
    {
    case 0:
        LoadPrimitive(rANCS);
        break;
    case 1:
    case 2:
        LoadBlend(rANCS);
        break;
    case 3:
        LoadRandom(rANCS);
        break;
    case 4:
        LoadSequence(rANCS);
        break;
    }
}

void CAnimSetLoader::LoadPrimitive(IInputStream& rANCS)
{
    u32 AnimID = rANCS.ReadLong();
    u32 AnimIndex = rANCS.ReadLong();
    TString AnimName = rANCS.ReadString();
    rANCS.Seek(0x8, SEEK_CUR);

    if (mAnimPrimitives.size() < (AnimIndex + 1))
        mAnimPrimitives.resize(AnimIndex + 1);

    if (!mAnimPrimitives[AnimIndex].Loaded)
    {
        mAnimPrimitives[AnimIndex].Index = AnimIndex;
        mAnimPrimitives[AnimIndex].AnimID = AnimID;
        mAnimPrimitives[AnimIndex].Name = AnimName;
        mAnimPrimitives[AnimIndex].Loaded = true;
    }
}

void CAnimSetLoader::LoadBlend(IInputStream& rANCS)
{
    LoadMetaAnimation(rANCS);
    LoadMetaAnimation(rANCS);
    rANCS.Seek(0x5, SEEK_CUR);
}

void CAnimSetLoader::LoadRandom(IInputStream& rANCS)
{
    u32 NumAnims = rANCS.ReadLong();

    for (u32 iAnim = 0; iAnim < NumAnims; iAnim++)
        LoadAnimProbabilityPair(rANCS);
}

void CAnimSetLoader::LoadAnimProbabilityPair(IInputStream& rANCS)
{
    LoadMetaAnimation(rANCS);
    rANCS.Seek(0x4, SEEK_CUR);
}

void CAnimSetLoader::LoadSequence(IInputStream& rANCS)
{
    u32 NumAnims = rANCS.ReadLong();

    for (u32 iAnim = 0; iAnim < NumAnims; iAnim++)
        LoadMetaAnimation(rANCS);
}

void CAnimSetLoader::LoadTransition(IInputStream& rANCS)
{
    rANCS.Seek(0xC, SEEK_CUR);
    LoadMetaTransition(rANCS);
}

void CAnimSetLoader::LoadMetaTransition(IInputStream& rANCS)
{
    u32 Type = rANCS.ReadLong();

    switch (Type)
    {
    case 0:
        LoadTransAnimation(rANCS);
        break;
    case 1:
    case 2:
        LoadTransTransition(rANCS);
        break;
    case 3:
        break;
    }
}

void CAnimSetLoader::LoadTransAnimation(IInputStream& rANCS)
{
    LoadMetaAnimation(rANCS);
}

void CAnimSetLoader::LoadTransTransition(IInputStream& rANCS)
{
    rANCS.Seek(0xE, SEEK_CUR);
}

void CAnimSetLoader::LoadAdditiveAnimation(IInputStream& rANCS)
{
    rANCS.Seek(0xC, SEEK_CUR);
}

void CAnimSetLoader::LoadHalfTransition(IInputStream& rANCS)
{
    rANCS.Seek(0x4, SEEK_CUR);
    LoadMetaTransition(rANCS);
}

// ************ STATIC ************
CAnimSet* CAnimSetLoader::LoadANCS(IInputStream& rANCS)
{
    if (!rANCS.IsValid()) return nullptr;

    u32 Magic = rANCS.ReadLong();
    if (Magic != 0x00010001)
    {
        Log::FileError(rANCS.GetSourceString(), "Invalid ANCS magic: " + TString::HexString(Magic));
        return nullptr;
    }

    CAnimSetLoader Loader;
    Loader.pSet = new CAnimSet;

    u32 NodeCount = rANCS.ReadLong();
    Loader.pSet->mNodes.resize(NodeCount);

    for (u32 iNode = 0; iNode < NodeCount; iNode++)
    {
        CAnimSet::SNode *pNode = &Loader.pSet->mNodes[iNode];

        rANCS.Seek(0x4, SEEK_CUR); // Skipping node self-index
        u16 Unknown1 = rANCS.ReadShort();
        if (iNode == 0) Loader.mVersion = (Unknown1 == 0xA) ? eEchoes : ePrime; // Best version indicator we know of unfortunately
        pNode->Name = rANCS.ReadString();
        pNode->pModel = gResCache.GetResource(rANCS.ReadLong(), "CMDL");

        if (Loader.mVersion <= ePrime)
        {
            pNode->pSkin = gResCache.GetResource(rANCS.ReadLong(), "CSKR");
            pNode->pSkeleton = gResCache.GetResource(rANCS.ReadLong(), "CINF");

            pNode->pModel->SetSkin(pNode->pSkin);
        }
        else
            rANCS.Seek(0x8, SEEK_CUR);

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
        rANCS.Seek(ParticleCount * 4, SEEK_CUR);
        u32 SwooshCount = rANCS.ReadLong();
        rANCS.Seek(SwooshCount * 4, SEEK_CUR);
        if (Unknown1 != 5) rANCS.Seek(0x4, SEEK_CUR);
        u32 ElectricCount = rANCS.ReadLong();
        rANCS.Seek(ElectricCount * 4, SEEK_CUR);

        if (Loader.mVersion == eEchoes)
        {
            u32 SPSCCount = rANCS.ReadLong();
            rANCS.Seek(SPSCCount * 4, SEEK_CUR);
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
                rANCS.Seek(0x8, SEEK_CUR);
                if (Loader.mVersion == ePrime) rANCS.ReadString();
                if (Loader.mVersion == eEchoes) rANCS.Seek(0x4, SEEK_CUR);
                rANCS.Seek(0xC, SEEK_CUR);
            }
        }
        rANCS.Seek(0x8, SEEK_CUR);

        u32 UnknownCount = rANCS.ReadLong();
        rANCS.Seek(UnknownCount * 4, SEEK_CUR);

        if (Loader.mVersion == eEchoes)
        {
            rANCS.Seek(0x5, SEEK_CUR);
            u32 UnknownCount2 = rANCS.ReadLong();
            rANCS.Seek(UnknownCount2 * 0x1C, SEEK_CUR);
        }
    }

    // Load Animation Set
    u32 SetStart = rANCS.Tell();
    SetStart = SetStart;
    u16 InfoCount = rANCS.ReadShort();
    u32 NumAnims = rANCS.ReadLong();

    for (u32 iAnim = 0; iAnim < NumAnims; iAnim++)
        Loader.LoadAnimation(rANCS);

    u32 NumTransitions = rANCS.ReadLong();

    for (u32 iTrans = 0; iTrans < NumTransitions; iTrans++)
        Loader.LoadTransition(rANCS);
    Loader.LoadMetaTransition(rANCS);

    u32 NumAdditiveAnims = rANCS.ReadLong();

    for (u32 iAnim = 0; iAnim < NumAdditiveAnims; iAnim++)
        Loader.LoadAdditiveAnimation(rANCS);

    rANCS.Seek(0x8, SEEK_CUR);

    if (InfoCount > 2)
    {
        u32 NumHalfTransitions = rANCS.ReadLong();

        for (u32 iHalf = 0; iHalf < NumHalfTransitions; iHalf++)
            Loader.LoadHalfTransition(rANCS);
    }

    // Add anims to set
    for (u32 iPrim = 0; iPrim < Loader.mAnimPrimitives.size(); iPrim++)
    {
        SPrimitive& rPrim = Loader.mAnimPrimitives[iPrim];

        if (rPrim.Loaded)
        {
            CAnimSet::SAnimation Anim;
            Anim.Name = rPrim.Name;
            Anim.pAnim = gResCache.GetResource(rPrim.AnimID, "ANIM");
            Loader.pSet->mAnims.push_back(Anim);
        }
    }

    return Loader.pSet;
}

CAnimSet* CAnimSetLoader::LoadCHAR(IInputStream& rCHAR)
{
    if (!rCHAR.IsValid()) return nullptr;

    CAnimSetLoader Loader;
    u8 Check = rCHAR.ReadByte();

    if (Check == 0x5 || Check == 0x3)
    {
        Loader.mVersion = eCorruption;
        Loader.pSet = new CAnimSet();
        return Loader.LoadCorruptionCHAR(rCHAR);
    }

    if (Check == 0x59)
    {
        Loader.mVersion = eReturns;
        Loader.pSet = new CAnimSet();
        return Loader.LoadReturnsCHAR(rCHAR);
    }

    Log::FileError(rCHAR.GetSourceString(), "CHAR has invalid first byte: " + TString::HexString(Check, 2));
    return nullptr;
}
