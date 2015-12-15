#include "CPointOfInterestExtra.h"

const CColor CPointOfInterestExtra::skRegularColor  ((u32) 0xFF7000FF);
const CColor CPointOfInterestExtra::skImportantColor((u32) 0xFF0000FF);

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
        mpScanProperty = (CFileProperty*) pBaseProp->PropertyByIDString("0x04:0x00");
        break;

    case eEchoesDemo:
    case eEchoes:
    case eCorruptionProto:
    case eCorruption:
        mpScanProperty = (CFileProperty*) pBaseProp->PropertyByIDString("0xBDBEC295:0xB94E9BE7");
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

void CPointOfInterestExtra::PropertyModified(CPropertyBase* pProperty)
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
