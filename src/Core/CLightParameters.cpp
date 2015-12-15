#include "CLightParameters.h"

CLightParameters::CLightParameters(CPropertyStruct *pStruct, EGame game)
    : mpStruct(pStruct), mGame(game)
{
}

CLightParameters::~CLightParameters()
{
}

int CLightParameters::LightLayerIndex()
{
    if (!mpStruct) return 0;

    CLongProperty *pParam;

    if (mGame <= ePrime)
        pParam = (CLongProperty*) mpStruct->PropertyByIndex(0xD);
    else
        pParam = (CLongProperty*) mpStruct->PropertyByID(0x1F715FD3);

    if (!pParam) return 0;
    else return pParam->Get();
}
