#include "CPointOfInterestExtra.h"

const CColor CPointOfInterestExtra::skRegularColor   = CColor::Integral(0xFF,0x70,0x00);
const CColor CPointOfInterestExtra::skImportantColor = CColor::Integral(0xFF,0x00,0x00);

CPointOfInterestExtra::CPointOfInterestExtra(CScriptObject *pInstance, CSceneManager *pScene, CSceneNode *pParent)
    : CScriptExtra(pInstance, pScene, pParent)
    , mpScanProperty(nullptr)
    , mpScanData(nullptr)
{
    // Fetch scan data property
    CPropertyStruct *pBaseProp = pInstance->Properties();

    switch (mGame)
    {
    case ePrimeDemo:
    case ePrime:
        mpScanProperty = (TFileProperty*) pBaseProp->PropertyByIDString("0x04:0x00");
        break;

    case eEchoesDemo:
    case eEchoes:
    case eCorruptionProto:
    case eCorruption:
        mpScanProperty = (TFileProperty*) pBaseProp->PropertyByIDString("0xBDBEC295:0xB94E9BE7");
        break;

    default:
        mpScanProperty = nullptr;
        break;
    }

    if (mpScanProperty)
    {
        if (mpScanProperty->Type() == eFileProperty)
            PropertyModified(mpScanProperty);
        else
            mpScanProperty = nullptr;
    }
}

void CPointOfInterestExtra::PropertyModified(IProperty* pProperty)
{
    if (mpScanProperty == pProperty)
        mpScanData = mpScanProperty->Get();
}

void CPointOfInterestExtra::ModifyTintColor(CColor& Color)
{
    if (mpScanData)
    {
        if (mpScanData->IsImportant()) Color *= skImportantColor;
        else Color *= skRegularColor;
    }
}
