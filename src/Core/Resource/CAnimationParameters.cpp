#include "CAnimationParameters.h"
#include "CAnimSet.h"
#include "CResourceInfo.h"
#include "Core/GameProject/CResourceStore.h"
#include <Common/Log.h>
#include <iostream>

CAnimationParameters::CAnimationParameters()
    : mGame(ePrime)
    , mCharIndex(0)
    , mAnimIndex(0)
    , mUnknown2(0)
    , mUnknown3(0)
{
}

CAnimationParameters::CAnimationParameters(EGame Game)
    : mGame(Game)
    , mCharIndex(0)
    , mAnimIndex(0)
    , mUnknown2(0)
    , mUnknown3(0)
{
}

CAnimationParameters::CAnimationParameters(IInputStream& rSCLY, EGame Game)
    : mGame(Game)
    , mCharIndex(0)
    , mAnimIndex(0)
    , mUnknown2(0)
    , mUnknown3(0)
{
    if (Game <= eEchoes)
    {
        mCharacter = CResourceInfo(rSCLY.ReadLong(), "ANCS");
        mCharIndex = rSCLY.ReadLong();
        mAnimIndex = rSCLY.ReadLong();
    }

    else if (Game <= eCorruption)
    {
        mCharacter = CResourceInfo(rSCLY.ReadLongLong(), "CHAR");
        mAnimIndex = rSCLY.ReadLong();
    }

    else if (Game == eReturns)
    {
        u8 Flags = rSCLY.ReadByte();

        // 0x80 - CharacterAnimationSet is empty.
        if (Flags & 0x80)
        {
            mAnimIndex = -1;
            mUnknown2 = 0;
            mUnknown3 = 0;
            return;
        }

        mCharacter = CResourceInfo(rSCLY.ReadLongLong(), "CHAR");

        // 0x20 - Default Anim is present
        if (Flags & 0x20)
            mAnimIndex = rSCLY.ReadLong();
        else
            mAnimIndex = -1;

        // 0x40 - Two-value struct is present
        if (Flags & 0x40)
        {
            mUnknown2 = rSCLY.ReadLong();
            mUnknown3 = rSCLY.ReadLong();
        }
        else
        {
            mUnknown2 = 0;
            mUnknown3 = 0;
        }
    }
}

void CAnimationParameters::Write(IOutputStream& rSCLY)
{
    if (mGame <= eEchoes)
    {
        if (mCharacter.IsValid())
        {
            rSCLY.WriteLong(mCharacter.ID().ToLong());
            rSCLY.WriteLong(mCharIndex);
            rSCLY.WriteLong(mAnimIndex);
        }
        else
        {
            rSCLY.WriteLong(0xFFFFFFFF);
            rSCLY.WriteLong(0);
            rSCLY.WriteLong(0xFFFFFFFF);
        }
    }

    else if (mGame <= eCorruption)
    {
        if (mCharacter.IsValid())
        {
            rSCLY.WriteLongLong(mCharacter.ID().ToLongLong());
            rSCLY.WriteLong(mAnimIndex);
        }

        else
        {
            rSCLY.WriteLongLong(CUniqueID::skInvalidID64.ToLongLong());
            rSCLY.WriteLong(0xFFFFFFFF);
        }
    }

    else
    {
        if (!mCharacter.IsValid())
            rSCLY.WriteByte((u8) 0x80);

        else
        {
            u8 Flag = 0;
            if (mAnimIndex != -1) Flag |= 0x20;
            if (mUnknown2 != 0 || mUnknown3 != 0) Flag |= 0x40;

            rSCLY.WriteByte(Flag);
            rSCLY.WriteLongLong(mCharacter.ID().ToLongLong());

            if (Flag & 0x20)
                rSCLY.WriteLong(mAnimIndex);

            if (Flag & 0x40)
            {
                rSCLY.WriteLong(mUnknown2);
                rSCLY.WriteLong(mUnknown3);
            }
        }
    }
}

CModel* CAnimationParameters::GetCurrentModel(s32 NodeIndex /*= -1*/)
{
    if (!mCharacter.IsValid()) return nullptr;

    CAnimSet *pSet = (CAnimSet*) mCharacter.Load();
    if (!pSet) return nullptr;
    if (pSet->Type() != eAnimSet) return nullptr;
    if (NodeIndex == -1) NodeIndex = mCharIndex;

    if (pSet->NumNodes() <= (u32) NodeIndex) return nullptr;
    return pSet->NodeModel(NodeIndex);
}

TString CAnimationParameters::GetCurrentCharacterName(s32 NodeIndex /*= -1*/)
{
    if (!mCharacter.IsValid()) return "";

    CAnimSet *pSet = (CAnimSet*) mCharacter.Load();
    if (!pSet) return "";
    if (pSet->Type() != eAnimSet) return "";
    if (NodeIndex == -1) NodeIndex = mCharIndex;

    if (pSet->NumNodes() <= (u32) NodeIndex) return "";
    return pSet->NodeName((u32) NodeIndex);
}

// ************ ACCESSORS ************
u32 CAnimationParameters::Unknown(u32 Index)
{
    // mAnimIndex isn't unknown, but I'm too lazy to move it because there's a lot
    // of UI stuff that depends on these functions atm for accessing and editing parameters.
    switch (Index)
    {
    case 0: return mAnimIndex;
    case 1: return mUnknown2;
    case 2: return mUnknown3;
    default: return 0;
    }
}

void CAnimationParameters::SetResource(CResourceInfo Res)
{
    if (Res.Type() == "ANCS" || Res.Type() == "CHAR")
    {
        mCharacter = Res;
        mCharIndex = 0;
    }
    else
        Log::Error("Resource with invalid type passed to CAnimationParameters: " + Res.ToString());
}

void CAnimationParameters::SetUnknown(u32 Index, u32 Value)
{
    switch (Index)
    {
    case 0: mAnimIndex = Value;
    case 1: mUnknown2 = Value;
    case 2: mUnknown3 = Value;
    }
}
