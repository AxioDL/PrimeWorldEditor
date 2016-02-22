#ifndef CANIMATIONPARAMETERS_H
#define CANIMATIONPARAMETERS_H

#include "CAnimSet.h"
#include "CResourceInfo.h"
#include "EGame.h"
#include "TResPtr.h"
#include "Core/Resource/Model/CModel.h"

class CAnimationParameters
{
    EGame mGame;
    CResourceInfo mCharacter;

    u32 mNodeIndex;
    u32 mUnknown1;
    u32 mUnknown2;
    u32 mUnknown3;

public:
    CAnimationParameters();
    CAnimationParameters(IInputStream& SCLY, EGame Game);
    void Write(IOutputStream& rSCLY);

    CModel* GetCurrentModel(s32 NodeIndex = -1);
    TString GetCurrentCharacterName(s32 NodeIndex = -1);

    // Getters
    EGame Version();
    CAnimSet* AnimSet();
    u32 CharacterIndex();
    u32 Unknown(u32 index);

    // Setters
    void SetResource(CResourceInfo Res);
    void SetNodeIndex(u32 Index);
    void SetUnknown(u32 Index, u32 Value);

    // Operators
    inline bool operator==(const CAnimationParameters& rkOther) const
    {
        return ( (mGame == rkOther.mGame) &&
                 (mCharacter == rkOther.mCharacter) &&
                 (mNodeIndex == rkOther.mNodeIndex) &&
                 (mUnknown1 == rkOther.mUnknown1) &&
                 (mUnknown2 == rkOther.mUnknown2) &&
                 (mUnknown3 == rkOther.mUnknown3) );
    }
};

#endif // CANIMATIONPARAMETERS_H
