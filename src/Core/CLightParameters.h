#ifndef CLIGHTPARAMETERS_H
#define CLIGHTPARAMETERS_H

#include "Core/Resource/Area/CGameArea.h"
#include "Core/Resource/Script/Property/Properties.h"

enum class EWorldLightingOptions
{
    Unknown1,
    NormalLighting,
    Unknown2,
    DisableWorldLighting,
};

class CLightParameters
{
    CIntRef mLightLayer;
    TEnumRef<EWorldLightingOptions> mWorldLightingOptions;

public:
    CLightParameters(CStructRef InStruct, EGame Game)
    {
        if (InStruct.IsValid())
        {
            if (Game <= EGame::Prime)
            {
                mWorldLightingOptions = TEnumRef<EWorldLightingOptions>(InStruct.DataPointer(), InStruct.Property()->ChildByIndex(0x7));
                mLightLayer = CIntRef(InStruct.DataPointer(), InStruct.Property()->ChildByIndex(0xD));
            }
            else
            {
                mWorldLightingOptions = TEnumRef<EWorldLightingOptions>(InStruct.DataPointer(), InStruct.Property()->ChildByID(0x6B5E7509));
                mLightLayer = CIntRef(InStruct.DataPointer(), InStruct.Property()->ChildByID(0x1F715FD3));
            }
        }
    }

    int LightLayerIndex() const
    {
        return mLightLayer.IsValid() ? mLightLayer.Get() : 0;
    }

    EWorldLightingOptions WorldLightingOptions() const
    {
        return mWorldLightingOptions.IsValid() ? mWorldLightingOptions.Get() : EWorldLightingOptions::NormalLighting;
    }
};

#endif // CLIGHTPARAMETERS_H
