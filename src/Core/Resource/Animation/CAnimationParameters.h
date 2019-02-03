#ifndef CANIMATIONPARAMETERS_H
#define CANIMATIONPARAMETERS_H

#include "Core/Resource/TResPtr.h"
#include <Common/EGame.h>

class CModel;

class CAnimationParameters
{
    EGame mGame;
    CAssetID mCharacterID;

    uint32 mCharIndex;
    uint32 mAnimIndex;
    uint32 mUnknown2;
    uint32 mUnknown3;

public:
    CAnimationParameters();
    CAnimationParameters(EGame Game);
    CAnimationParameters(IInputStream& rSCLY, EGame Game);
    void Write(IOutputStream& rSCLY);
    void Serialize(IArchive& rArc);

    const SSetCharacter* GetCurrentSetCharacter(int32 NodeIndex = -1);
    CModel* GetCurrentModel(int32 NodeIndex = -1);
    TString GetCurrentCharacterName(int32 NodeIndex = -1);

    // Accessors
    inline EGame Version() const            { return mGame; }
    inline CAssetID ID() const              { return mCharacterID; }
    inline CAnimSet* AnimSet() const        { return (CAnimSet*) gpResourceStore->LoadResource(mCharacterID); }
    inline uint32 CharacterIndex() const    { return mCharIndex; }
    inline uint32 AnimIndex() const         { return mAnimIndex; }
    inline void SetCharIndex(uint32 Index)  { mCharIndex = Index; }
    inline void SetAnimIndex(uint32 Index)  { mAnimIndex = Index; }

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

    uint32 Unknown(uint32 Index);
    void SetResource(const CAssetID& rkID);
    void SetUnknown(uint32 Index, uint32 Value);

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
