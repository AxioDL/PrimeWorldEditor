#ifndef CLIGHTPARAMETERS_H
#define CLIGHTPARAMETERS_H

#include "Core/Resource/CGameArea.h"
#include "Core/Resource/Script/CProperty.h"

class CLightParameters
{
    CPropertyStruct *mpStruct;
    EGame mGame;

public:
    CLightParameters(CPropertyStruct *pStruct, EGame game);
    ~CLightParameters();
    int LightLayerIndex();
};

#endif // CLIGHTPARAMETERS_H
