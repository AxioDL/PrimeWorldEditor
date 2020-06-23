#include "CAnimationParameters.h"
#include "CAnimSet.h"
#include "Core/GameProject/CResourceStore.h"
#include <Common/Log.h>

CAnimationParameters::CAnimationParameters() = default;

CAnimationParameters::CAnimationParameters(EGame Game)
    : mGame(Game), mCharacterID(CAssetID::InvalidID(Game))
{
}

CAnimationParameters::CAnimationParameters(IInputStream& rSCLY, EGame Game)
    : mGame(Game)
{
    if (Game <= EGame::Echoes)
    {
        mCharacterID = CAssetID(rSCLY, Game);
        mCharIndex = rSCLY.ReadLong();
        mAnimIndex = rSCLY.ReadLong();
    }

    else if (Game <= EGame::Corruption)
    {
        mCharacterID = CAssetID(rSCLY, Game);
        mAnimIndex = rSCLY.ReadLong();
    }

    else if (Game == EGame::DKCReturns)
    {
        uint8 Flags = rSCLY.ReadByte();

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
    if (mGame <= EGame::Echoes)
    {
        if (mCharacterID.IsValid())
        {
            mCharacterID.Write(rSCLY);
            rSCLY.WriteLong(mCharIndex);
            rSCLY.WriteLong(mAnimIndex);
        }
        else
        {
            CAssetID::skInvalidID32.Write(rSCLY);
            rSCLY.WriteLong(0);
            rSCLY.WriteLong(mGame >= EGame::EchoesDemo ? 0 : 0xFFFFFFFF);
        }
    }

    else if (mGame <= EGame::Corruption)
    {
        if (mCharacterID.IsValid())
        {
            mCharacterID.Write(rSCLY);
            rSCLY.WriteLong(mAnimIndex);
        }

        else
        {
            CAssetID::skInvalidID64.Write(rSCLY);
            rSCLY.WriteLong(0xFFFFFFFF);
        }
    }

    else
    {
        if (!mCharacterID.IsValid())
            rSCLY.WriteByte((uint8) 0x80);

        else
        {
            uint8 Flag = 0;
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

void CAnimationParameters::Serialize(IArchive& rArc)
{
    if (rArc.IsReader())
        mGame = rArc.Game();

    rArc << SerialParameter("AnimationSetAsset", mCharacterID);

    if (mGame <= EGame::Echoes)
        rArc << SerialParameter("CharacterID", mCharIndex);

    rArc << SerialParameter("AnimationID", mAnimIndex);

    if (mGame >= EGame::DKCReturns)
    {
        rArc << SerialParameter("Unknown0", mUnknown2)
             << SerialParameter("Unknown1", mUnknown3);
    }
}

const SSetCharacter* CAnimationParameters::GetCurrentSetCharacter(int32 NodeIndex) const
{
    const CAnimSet *pSet = AnimSet();

    if (pSet != nullptr && (pSet->Type() == EResourceType::AnimSet || pSet->Type() == EResourceType::Character))
    {
        if (NodeIndex == -1)
            NodeIndex = mCharIndex;

        if (mCharIndex != UINT32_MAX && pSet->NumCharacters() > static_cast<uint32>(NodeIndex))
            return pSet->Character(NodeIndex);
    }

    return nullptr;
}

CModel* CAnimationParameters::GetCurrentModel(int32 NodeIndex)
{
    const SSetCharacter *pkChar = GetCurrentSetCharacter(NodeIndex);
    return pkChar != nullptr ? pkChar->pModel : nullptr;
}

TString CAnimationParameters::GetCurrentCharacterName(int32 NodeIndex) const
{
    const SSetCharacter *pkChar = GetCurrentSetCharacter(NodeIndex);
    return pkChar != nullptr ? pkChar->Name : "";
}

// ************ ACCESSORS ************
uint32 CAnimationParameters::Unknown(uint32 Index) const
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
    if (mCharacterID != rkID)
    {
        mCharacterID = rkID;
        mCharIndex = 0;
        mAnimIndex = 0;
        mUnknown2 = 0;
        mUnknown3 = 0;

        // Validate ID
        if (mCharacterID.IsValid())
        {
            CResourceEntry *pEntry = gpResourceStore->FindEntry(rkID);

            if (!pEntry)
                errorf("Invalid resource ID passed to CAnimationParameters: %s", *rkID.ToString());
            else if (pEntry->ResourceType() != EResourceType::AnimSet && pEntry->ResourceType() != EResourceType::Character)
                errorf("Resource with invalid type passed to CAnimationParameters: %s", *pEntry->CookedAssetPath().GetFileName());
        }
    }
}

void CAnimationParameters::SetUnknown(uint32 Index, uint32 Value)
{
    switch (Index)
    {
    case 0:
        mAnimIndex = Value;
        break;
    case 1:
        mUnknown2 = Value;
        break;
    case 2:
        mUnknown3 = Value;
        break;
    }
}
