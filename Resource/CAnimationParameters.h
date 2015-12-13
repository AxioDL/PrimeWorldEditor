#ifndef CANIMATIONPARAMETERS_H
#define CANIMATIONPARAMETERS_H

#include "CResource.h"
#include "model/CModel.h"
#include <Core/TResPtr.h>
#include "EFormatVersion.h"

class CAnimationParameters
{
    EGame mGame;
    TResPtr<CResource> mpCharSet;

    u32 mNodeIndex;
    u32 mUnknown1;
    u32 mUnknown2;
    u32 mUnknown3;
    u32 mUnknown4;

public:
    CAnimationParameters();
    CAnimationParameters(CInputStream& SCLY, EGame game);
    CModel* GetCurrentModel(s32 nodeIndex = -1);

    // Getters
    EGame Version();
    CResource* Resource();
    u32 CharacterIndex();
    u32 Unknown(u32 index);

    // Setters
    void SetResource(CResource *pRes);
    void SetNodeIndex(u32 index);
    void SetUnknown(u32 index, u32 value);
};

#endif // CANIMATIONPARAMETERS_H
