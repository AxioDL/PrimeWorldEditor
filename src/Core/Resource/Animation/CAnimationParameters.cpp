#include "Core/Resource/Animation/CAnimationParameters.h"

#include "Core/GameProject/CResourceEntry.h"
#include "Core/Resource/Animation/CAnimSet.h"
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
        mCharIndex = rSCLY.ReadU32();
        mAnimIndex = rSCLY.ReadU32();
    }
    else if (Game <= EGame::Corruption)
    {
        mCharacterID = CAssetID(rSCLY, Game);
        mAnimIndex = rSCLY.ReadU32();
    }
    else if (Game == EGame::DKCReturns)
    {
        const auto Flags = rSCLY.ReadU8();

        // 0x80 - CharacterAnimationSet is empty.
        if ((Flags & 0x80) != 0)
        {
            mAnimIndex = -1;
            mUnknown2 = 0;
            mUnknown3 = 0;
            return;
        }

        mCharacterID = CAssetID(rSCLY, Game);

        // 0x20 - Default Anim is present
        if ((Flags & 0x20) != 0)
            mAnimIndex = rSCLY.ReadU32();
        else
            mAnimIndex = -1;

        // 0x40 - Two-value struct is present
        if ((Flags & 0x40) != 0)
        {
            mUnknown2 = rSCLY.ReadU32();
            mUnknown3 = rSCLY.ReadU32();
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
            rSCLY.WriteU32(mCharIndex);
            rSCLY.WriteU32(mAnimIndex);
        }
        else
        {
            CAssetID::skInvalidID32.Write(rSCLY);
            rSCLY.WriteU32(0);
            rSCLY.WriteU32(mGame >= EGame::EchoesDemo ? 0 : 0xFFFFFFFF);
        }
    }
    else if (mGame <= EGame::Corruption)
    {
        if (mCharacterID.IsValid())
        {
            mCharacterID.Write(rSCLY);
            rSCLY.WriteU32(mAnimIndex);
        }

        else
        {
            CAssetID::skInvalidID64.Write(rSCLY);
            rSCLY.WriteU32(0xFFFFFFFF);
        }
    }
    else
    {
        if (!mCharacterID.IsValid())
        {
            rSCLY.WriteU8(uint8_t{0x80});
        }
        else
        {
            uint8_t Flag = 0;
            if (mAnimIndex != -1)
                Flag |= 0x20;
            if (mUnknown2 != 0 || mUnknown3 != 0)
                Flag |= 0x40;

            rSCLY.WriteU8(Flag);
            mCharacterID.Write(rSCLY);

            if ((Flag & 0x20) != 0)
                rSCLY.WriteU32(mAnimIndex);

            if ((Flag & 0x40) != 0)
            {
                rSCLY.WriteU32(mUnknown2);
                rSCLY.WriteU32(mUnknown3);
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

const SSetCharacter* CAnimationParameters::GetCurrentSetCharacter(int32_t NodeIndex) const
{
    const CAnimSet *pSet = AnimSet();

    if (pSet != nullptr && (pSet->Type() == EResourceType::AnimSet || pSet->Type() == EResourceType::Character))
    {
        if (NodeIndex == -1)
            NodeIndex = mCharIndex;

        if (mCharIndex != UINT32_MAX && pSet->NumCharacters() > static_cast<uint32_t>(NodeIndex))
            return pSet->Character(NodeIndex);
    }

    return nullptr;
}

CModel* CAnimationParameters::GetCurrentModel(int32_t NodeIndex)
{
    const SSetCharacter *pkChar = GetCurrentSetCharacter(NodeIndex);
    return pkChar != nullptr ? pkChar->pModel : nullptr;
}

TString CAnimationParameters::GetCurrentCharacterName(int32_t NodeIndex) const
{
    const SSetCharacter *pkChar = GetCurrentSetCharacter(NodeIndex);
    return pkChar != nullptr ? pkChar->Name : "";
}

// ************ ACCESSORS ************
uint32_t CAnimationParameters::Unknown(uint32_t Index) const
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

void CAnimationParameters::SetResource(const CResourceEntry* entry)
{
    if (entry != nullptr && mCharacterID == entry->ID())
        return;

    mCharacterID = entry != nullptr ? entry->ID() : CAssetID::skInvalidID32;
    mCharIndex = 0;
    mAnimIndex = 0;
    mUnknown2 = 0;
    mUnknown3 = 0;

    // Validate ID
    if (mCharacterID.IsValid())
    {
        if (entry->ResourceType() != EResourceType::AnimSet && entry->ResourceType() != EResourceType::Character)
            NLog::Error("Resource with invalid type passed to CAnimationParameters: {}", entry->CookedAssetPath().GetFileName());
    }

}

void CAnimationParameters::SetUnknown(uint32_t Index, uint32_t Value)
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
