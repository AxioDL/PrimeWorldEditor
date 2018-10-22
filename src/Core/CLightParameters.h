#ifndef CLIGHTPARAMETERS_H
#define CLIGHTPARAMETERS_H

#include "Core/Resource/Area/CGameArea.h"
#include "Core/Resource/Script/Property/Properties.h"

enum EWorldLightingOptions
{
    eUnknown1 = 0,
    eNormalLighting = 1,
    eUnknown2 = 2,
    eDisableWorldLighting = 3
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

    inline int LightLayerIndex() const
    {
        return mLightLayer.IsValid() ? mLightLayer.Get() : 0;
    }

    inline EWorldLightingOptions WorldLightingOptions() const
    {
        return mWorldLightingOptions.IsValid() ? mWorldLightingOptions.Get() : eNormalLighting;
    }
};

#endif // CLIGHTPARAMETERS_H
