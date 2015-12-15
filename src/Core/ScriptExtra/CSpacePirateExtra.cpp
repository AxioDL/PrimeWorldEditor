#include "CSpacePirateExtra.h"

CSpacePirateExtra::CSpacePirateExtra(CScriptObject *pInstance, CSceneManager *pScene, CSceneNode *pParent)
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
        mpPowerVuln = (CLongProperty*) pVulns->PropertyByID(0x0);
        if (mpPowerVuln && mpPowerVuln->Type() != eLongProperty && mpPowerVuln->Type() != eEnumProperty)
            mpPowerVuln = nullptr;

        mpWaveVuln = (CLongProperty*) pVulns->PropertyByID(0x2);
        if (mpWaveVuln && mpWaveVuln->Type() != eLongProperty && mpWaveVuln->Type() != eEnumProperty)
            mpWaveVuln = nullptr;

        mpIceVuln = (CLongProperty*) pVulns->PropertyByID(0x1);
        if (mpIceVuln && mpIceVuln->Type() != eLongProperty && mpIceVuln->Type() != eEnumProperty)
            mpIceVuln = nullptr;

        mpPlasmaVuln = (CLongProperty*) pVulns->PropertyByID(0x3);
        if (mpPlasmaVuln && mpPlasmaVuln->Type() != eLongProperty && mpPlasmaVuln->Type() != eEnumProperty)
            mpPlasmaVuln = nullptr;
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
