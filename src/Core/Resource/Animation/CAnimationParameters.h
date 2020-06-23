#ifndef CANIMATIONPARAMETERS_H
#define CANIMATIONPARAMETERS_H

#include "Core/Resource/TResPtr.h"
#include <Common/EGame.h>

class CModel;

class CAnimationParameters
{
    EGame mGame = EGame::Prime;
    CAssetID mCharacterID;

    uint32 mCharIndex = 0;
    uint32 mAnimIndex = 0;
    uint32 mUnknown2 = 0;
    uint32 mUnknown3 = 0;

public:
    CAnimationParameters();
    explicit CAnimationParameters(EGame Game);
    explicit CAnimationParameters(IInputStream& rSCLY, EGame Game);

    CAnimationParameters(const CAnimationParameters&) = default;
    CAnimationParameters& operator=(const CAnimationParameters&) = default;

    void Write(IOutputStream& rSCLY);
    void Serialize(IArchive& rArc);

    const SSetCharacter* GetCurrentSetCharacter(int32 NodeIndex = -1) const;
    CModel* GetCurrentModel(int32 NodeIndex = -1);
    TString GetCurrentCharacterName(int32 NodeIndex = -1) const;

    // Accessors
    EGame Version() const            { return mGame; }
    CAssetID ID() const              { return mCharacterID; }
    CAnimSet* AnimSet() const        { return (CAnimSet*) gpResourceStore->LoadResource(mCharacterID); }
    uint32 CharacterIndex() const    { return mCharIndex; }
    uint32 AnimIndex() const         { return mAnimIndex; }
    void SetCharIndex(uint32 Index)  { mCharIndex = Index; }
    void SetAnimIndex(uint32 Index)  { mAnimIndex = Index; }

    void SetGame(EGame Game)
    {
        mGame = Game;

        if (!mCharacterID.IsValid())
        {
            mCharacterID = CAssetID::InvalidID(mGame);
        }
        else
        {
            ASSERT( mCharacterID.Length() == CAssetID::GameIDLength(mGame) );
        }
    }

    uint32 Unknown(uint32 Index) const;
    void SetResource(const CAssetID& rkID);
    void SetUnknown(uint32 Index, uint32 Value);

    bool operator==(const CAnimationParameters& other) const
    {
        return mGame == other.mGame &&
               mCharacterID == other.mCharacterID &&
               mCharIndex == other.mCharIndex &&
               mAnimIndex == other.mAnimIndex &&
               mUnknown2 == other.mUnknown2 &&
               mUnknown3 == other.mUnknown3;
    }
    bool operator!=(const CAnimationParameters& other) const
    {
        return !operator==(other);
    }
};

#endif // CANIMATIONPARAMETERS_H
