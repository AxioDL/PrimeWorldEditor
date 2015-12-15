#ifndef CLIGHTPARAMETERS_H
#define CLIGHTPARAMETERS_H

#include <Resource/CGameArea.h>
#include <Resource/script/CProperty.h>

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
