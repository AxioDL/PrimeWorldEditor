#ifndef CANIMATIONPARAMETERS_H
#define CANIMATIONPARAMETERS_H

#include "Core/Resource/TResPtr.h"
#include "Core/Resource/Model/CModel.h"
#include <Common/EGame.h>

class CAnimationParameters
{
    EGame mGame;
    CAssetID mCharacterID;

    u32 mCharIndex;
    u32 mAnimIndex;
    u32 mUnknown2;
    u32 mUnknown3;

public:
    CAnimationParameters();
    CAnimationParameters(EGame Game);
    CAnimationParameters(IInputStream& rSCLY, EGame Game);
    void Write(IOutputStream& rSCLY);
    void Serialize(IArchive& rArc);

    const SSetCharacter* GetCurrentSetCharacter(s32 NodeIndex = -1);
    CModel* GetCurrentModel(s32 NodeIndex = -1);
    TString GetCurrentCharacterName(s32 NodeIndex = -1);

    // Accessors
    inline EGame Version() const        { return mGame; }
    inline CAssetID ID() const          { return mCharacterID; }
    inline CAnimSet* AnimSet() const    { return (CAnimSet*) gpResourceStore->LoadResource(mCharacterID); }
    inline u32 CharacterIndex() const   { return mCharIndex; }
    inline u32 AnimIndex() const        { return mAnimIndex; }
    inline void SetCharIndex(u32 Index) { mCharIndex = Index; }
    inline void SetAnimIndex(u32 Index) { mAnimIndex = Index; }

    inline void SetGame(EGame Game)
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

    u32 Unknown(u32 Index);
    void SetResource(const CAssetID& rkID);
    void SetUnknown(u32 Index, u32 Value);

    // Operators
    inline CAnimationParameters& operator=(const CAnimationParameters& rkOther)
    {
        mGame = rkOther.mGame;
        mCharacterID = rkOther.mCharacterID;
        mCharIndex = rkOther.mCharIndex;
        mAnimIndex = rkOther.mAnimIndex;
        mUnknown2 = rkOther.mUnknown2;
        mUnknown3 = rkOther.mUnknown3;
        return *this;
    }

    inline bool operator==(const CAnimationParameters& rkOther) const
    {
        return ( (mGame == rkOther.mGame) &&
                 (mCharacterID == rkOther.mCharacterID) &&
                 (mCharIndex == rkOther.mCharIndex) &&
                 (mAnimIndex == rkOther.mAnimIndex) &&
                 (mUnknown2 == rkOther.mUnknown2) &&
                 (mUnknown3 == rkOther.mUnknown3) );
    }
};

#endif // CANIMATIONPARAMETERS_H
