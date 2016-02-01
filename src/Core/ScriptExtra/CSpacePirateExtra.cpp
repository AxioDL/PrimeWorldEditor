#include "CSpacePirateExtra.h"

CSpacePirateExtra::CSpacePirateExtra(CScriptObject *pInstance, CScene *pScene, CSceneNode *pParent)
    : CScriptExtra(pInstance, pScene ,pParent)
    , mpPowerVuln(nullptr)
    , mpWaveVuln(nullptr)
    , mpIceVuln(nullptr)
    , mpPlasmaVuln(nullptr)
{
    CPropertyStruct *pBaseStruct = pInstance->Properties();
    CPropertyStruct *pVulns = (CPropertyStruct*) pBaseStruct->PropertyByIDString("0x04:0x10");

    if (pVulns && pVulns->Type() == eStructProperty)
    {
        mpPowerVuln = TPropCast<TEnumProperty>(pVulns->PropertyByID(0x0));
        mpWaveVuln = TPropCast<TEnumProperty>(pVulns->PropertyByID(0x2));
        mpIceVuln = TPropCast<TEnumProperty>(pVulns->PropertyByID(0x1));
        mpPlasmaVuln = TPropCast<TEnumProperty>(pVulns->PropertyByID(0x3));
    }
}

CColor CSpacePirateExtra::TevColor()
{
    // Priority: Plasma -> Ice -> Power -> Wave
    if (mpPlasmaVuln && mpPlasmaVuln->Get() == 1)
        return CColor::skRed;

    if (mpIceVuln && mpIceVuln->Get() == 1)
        return CColor::skWhite;

    if (mpPowerVuln && mpPowerVuln->Get() == 1)
        return CColor::skYellow;

    if (mpWaveVuln && mpWaveVuln->Get() == 1)
        return CColor::skPurple;

    return CColor::skWhite;
}
