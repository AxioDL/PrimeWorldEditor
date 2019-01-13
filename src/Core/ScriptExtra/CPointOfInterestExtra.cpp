#include "CPointOfInterestExtra.h"

//@todo pull these values from tweaks instead of hardcoding them
const CColor CPointOfInterestExtra::skRegularColor   = CColor::Integral(0xFF,0x70,0x00);
const CColor CPointOfInterestExtra::skImportantColor = CColor::Integral(0xFF,0x00,0x00);

CPointOfInterestExtra::CPointOfInterestExtra(CScriptObject *pInstance, CScene *pScene, CScriptNode *pParent)
    : CScriptExtra(pInstance, pScene, pParent)
    , mpScanData(nullptr)
{
    // Fetch scan data property
    CStructProperty* pProperties = pInstance->Template()->Properties();

    if (mGame <= EGame::Prime)  mScanProperty = CAssetRef(pInstance->PropertyData(), pProperties->ChildByIDString("0x04:0x00"));
    else                        mScanProperty = CAssetRef(pInstance->PropertyData(), pProperties->ChildByIDString("0xBDBEC295:0xB94E9BE7"));

    PropertyModified(mScanProperty.Property());
}

void CPointOfInterestExtra::PropertyModified(IProperty* pProperty)
{
    if (mScanProperty.Property() == pProperty)
    {
        mpScanData = gpResourceStore->LoadResource<CScan>( mScanProperty.Get() );
        mScanIsCritical = (mpScanData ? mpScanData->IsCriticalPropertyRef() : CBoolRef());
    }
}

void CPointOfInterestExtra::ModifyTintColor(CColor& Color)
{
    if (mpScanData)
    {
        if (mScanIsCritical) Color *= skImportantColor;
        else Color *= skRegularColor;
    }
}
