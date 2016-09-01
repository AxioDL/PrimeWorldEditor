#include "CAnimationParameters.h"
#include "CAnimSet.h"
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
        mCharacterID = CAssetID(rSCLY, Game);
        mCharIndex = rSCLY.ReadLong();
        mAnimIndex = rSCLY.ReadLong();
    }

    else if (Game <= eCorruption)
    {
        mCharacterID = CAssetID(rSCLY, Game);
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

        mCharacterID = CAssetID(rSCLY, Game);

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
        if (mCharacterID.IsValid())
        {
            mCharacterID.Write(rSCLY);
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
        if (mCharacterID.IsValid())
        {
            mCharacterID.Write(rSCLY);
            rSCLY.WriteLong(mAnimIndex);
        }

        else
        {
            rSCLY.WriteLongLong(CAssetID::skInvalidID64.ToLongLong());
            rSCLY.WriteLong(0xFFFFFFFF);
        }
    }

    else
    {
        if (!mCharacterID.IsValid())
            rSCLY.WriteByte((u8) 0x80);

        else
        {
            u8 Flag = 0;
            if (mAnimIndex != -1) Flag |= 0x20;
            if (mUnknown2 != 0 || mUnknown3 != 0) Flag |= 0x40;

            rSCLY.WriteByte(Flag);
            mCharacterID.Write(rSCLY);

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
    if (!mCharacterID.IsValid()) return nullptr;

    CAnimSet *pSet = AnimSet();
    if (!pSet) return nullptr;
    if (pSet->Type() != eAnimSet) return nullptr;
    if (NodeIndex == -1) NodeIndex = mCharIndex;

    if (pSet->NumNodes() <= (u32) NodeIndex) return nullptr;
    return pSet->NodeModel(NodeIndex);
}

TString CAnimationParameters::GetCurrentCharacterName(s32 NodeIndex /*= -1*/)
{
    if (!mCharacterID.IsValid()) return "";

    CAnimSet *pSet = AnimSet();
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

void CAnimationParameters::SetResource(const CAssetID& rkID)
{
    mCharacterID = rkID;
    mCharIndex = 0;
    mAnimIndex = 0;

    // Validate ID
    if (mCharacterID.IsValid())
    {
        CResourceEntry *pEntry = gpResourceStore->FindEntry(rkID);

        if (!pEntry)
            Log::Error("Invalid resource ID passed to CAnimationParameters: " + rkID.ToString());
        else if (pEntry->ResourceType() != eAnimSet)
            Log::Error("Resource with invalid type passed to CAnimationParameters: " + pEntry->CookedAssetPath().GetFileName());
    }
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
