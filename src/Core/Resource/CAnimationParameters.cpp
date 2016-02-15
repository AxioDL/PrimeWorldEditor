#include "CAnimationParameters.h"
#include "CAnimSet.h"
#include "CResCache.h"
#include <Common/Log.h>
#include <iostream>

CAnimationParameters::CAnimationParameters()
{
    mGame = ePrime;
    mpCharSet = nullptr;
    mNodeIndex = 0;
    mUnknown1 = 0;
    mUnknown2 = 0;
    mUnknown3 = 0;
    mUnknown4 = 0;
}

CAnimationParameters::CAnimationParameters(IInputStream& SCLY, EGame Game)
{
    mGame = Game;
    mpCharSet = nullptr;
    mNodeIndex = 0;
    mUnknown1 = 0;
    mUnknown2 = 0;
    mUnknown3 = 0;
    mUnknown4 = 0;

    if (Game <= eEchoes)
    {
        u32 AnimSetID = SCLY.ReadLong();
        mNodeIndex = SCLY.ReadLong();
        mUnknown1 = SCLY.ReadLong();

        mpCharSet = gResCache.GetResource(AnimSetID, "ANCS");
    }

    else if (Game <= eCorruption)
    {
        u64 CharID = SCLY.ReadLongLong();
        mUnknown1 = SCLY.ReadLong();

        mpCharSet = gResCache.GetResource(CharID, "CHAR");
    }

    else if (Game == eReturns)
    {
        SCLY.Seek(-6, SEEK_CUR);
        u32 Offset = SCLY.Tell();
        u32 PropID = SCLY.ReadLong();
        SCLY.Seek(2, SEEK_CUR);

        mUnknown1 = (u32) SCLY.ReadByte();
        mUnknown1 &= 0xFF;

        if (mUnknown1 == 0x60)
        {
            u64 charID = SCLY.ReadLongLong();
            mUnknown2 = SCLY.ReadLong();
            mUnknown3 = SCLY.ReadLong();
            mUnknown4 = SCLY.ReadLong();

            mpCharSet = gResCache.GetResource(charID, "CHAR");
        }

        else if (mUnknown1 != 0x80)
        {
            Log::FileError(SCLY.GetSourceString(), Offset,
                           "Unexpected AnimationParameters byte: " + TString::HexString(mUnknown1, true, true, 2) + " (property " + TString::HexString(PropID, true, true, 8) + ")");
        }
    }
}

void CAnimationParameters::Write(IOutputStream& rSCLY)
{
    if (mGame <= eEchoes)
    {
        if (mpCharSet)
        {
            rSCLY.WriteLong(AnimSet()->ResID().ToLong());
            rSCLY.WriteLong(mNodeIndex);
            rSCLY.WriteLong(mUnknown1);
        }
        else
        {
            rSCLY.WriteLong(0xFFFFFFFF);
            rSCLY.WriteLong(0);
            rSCLY.WriteLong(0xFFFFFFFF);
        }
    }
}

CModel* CAnimationParameters::GetCurrentModel(s32 NodeIndex /*= -1*/)
{
    if (!mpCharSet) return nullptr;
    if (mpCharSet->Type() != eAnimSet) return nullptr;
    if (NodeIndex == -1) NodeIndex = mNodeIndex;

    if (mpCharSet->getNodeCount() <= (u32) NodeIndex) return nullptr;
    return mpCharSet->getNodeModel(NodeIndex);
}

TString CAnimationParameters::GetCurrentCharacterName(s32 NodeIndex /*= -1*/)
{
    if (!mpCharSet) return "";
    if (mpCharSet->Type() != eAnimSet) return "";
    if (NodeIndex == -1) NodeIndex = mNodeIndex;

    if (mpCharSet->getNodeCount() <= (u32) NodeIndex) return "";
    return mpCharSet->getNodeName((u32) NodeIndex);
}

// ************ GETTERS ************
EGame CAnimationParameters::Version()
{
    return mGame;
}

CAnimSet* CAnimationParameters::AnimSet()
{
    return mpCharSet;
}

u32 CAnimationParameters::CharacterIndex()
{
    return mNodeIndex;
}

u32 CAnimationParameters::Unknown(u32 Index)
{
    switch (Index)
    {
    case 0: return mUnknown1;
    case 1: return mUnknown2;
    case 2: return mUnknown3;
    case 3: return mUnknown4;
    default: return 0;
    }
}

// ************ SETTERS ************
void CAnimationParameters::SetResource(CResource *pRes)
{
    if (!pRes || (pRes->Type() == eAnimSet) || (pRes->Type() == eCharacter))
    {
        mpCharSet = pRes;
        mNodeIndex = 0;
    }
    else
        Log::Error("Resource with invalid type passed to CAnimationParameters: " + pRes->Source());
}

void CAnimationParameters::SetNodeIndex(u32 Index)
{
    mNodeIndex = Index;
}

void CAnimationParameters::SetUnknown(u32 Index, u32 Value)
{
    switch (Index)
    {
    case 0: mUnknown1 = Value;
    case 1: mUnknown2 = Value;
    case 2: mUnknown3 = Value;
    case 3: mUnknown4 = Value;
    }
}
