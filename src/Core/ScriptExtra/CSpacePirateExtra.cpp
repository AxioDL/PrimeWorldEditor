#include "CSpacePirateExtra.h"

CSpacePirateExtra::CSpacePirateExtra(CScriptObject* pInstance, CScene* pScene, CScriptNode* pParent)
    : CScriptExtra(pInstance, pScene ,pParent)
{
    CStructProperty* pBaseStruct = pInstance->Template()->Properties();
    CStructProperty* pVulnerabilities = TPropCast<CStructProperty>(pBaseStruct->ChildByIDString("0x04:0x10"));

    if (pVulnerabilities)
    {
        mPowerVulnerability     = TEnumRef<EVulnerabilityTypeMP1>(pInstance, pVulnerabilities->ChildByID(0));
        mWaveVulnerability      = TEnumRef<EVulnerabilityTypeMP1>(pInstance, pVulnerabilities->ChildByID(2));
        mIceVulnerability       = TEnumRef<EVulnerabilityTypeMP1>(pInstance, pVulnerabilities->ChildByID(1));
        mPlasmaVulnerability    = TEnumRef<EVulnerabilityTypeMP1>(pInstance, pVulnerabilities->ChildByID(3));
    }
}

CColor CSpacePirateExtra::TevColor()
{
    // Priority: Plasma -> Ice -> Power -> Wave
    if (mPlasmaVulnerability.IsValid() && mPlasmaVulnerability.Get() == EVulnerabilityTypeMP1::Normal)
        return CColor::Red();

    if (mIceVulnerability.IsValid() && mIceVulnerability.Get() == EVulnerabilityTypeMP1::Normal)
        return CColor::White();

    if (mPowerVulnerability.IsValid() && mPowerVulnerability.Get() == EVulnerabilityTypeMP1::Normal)
        return CColor::Yellow();

    if (mWaveVulnerability.IsValid() && mWaveVulnerability.Get() == EVulnerabilityTypeMP1::Normal)
        return CColor::Purple();

    return CColor::White();
}
