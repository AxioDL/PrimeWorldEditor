#include "CSandwormExtra.h"

CSandwormExtra::CSandwormExtra(CScriptObject *pInstance, CScene *pScene, CScriptNode *pParent)
    : CScriptExtra(pInstance, pScene, pParent)
{
    // The back pincers need to be flipped 180 degrees
    for (u32 iAttach = 0; iAttach < pParent->NumAttachments(); iAttach++)
    {
        CScriptAttachNode *pAttach = pParent->Attachment(iAttach);

        if (pAttach->LocatorName() == "L_back_claw" || pAttach->LocatorName() == "R_back_claw")
            pAttach->SetRotation(CVector3f(0,0,180));
    }

    // Get pincers scale
    mpPincersScaleProperty = TPropCast<TFloatProperty>(pInstance->PropertyByIDString("0x3DB583AE"));
    if (mpPincersScaleProperty) PropertyModified(mpPincersScaleProperty);
}

void CSandwormExtra::PropertyModified(IProperty *pProp)
{
    if (pProp == mpPincersScaleProperty)
    {
        for (u32 iAttach = 0; iAttach < mpScriptNode->NumAttachments(); iAttach++)
        {
            CScriptAttachNode *pAttach = mpScriptNode->Attachment(iAttach);
            pAttach->SetScale(CVector3f(mpPincersScaleProperty->Get()));
        }
    }
}
