#include "CSandwormExtra.h"

CSandwormExtra::CSandwormExtra(CScriptObject* pInstance, CScene* pScene, CScriptNode* pParent)
    : CScriptExtra(pInstance, pScene, pParent)
{
    // The back pincers need to be flipped 180 degrees
    for (u32 AttachIdx = 0; AttachIdx < pParent->NumAttachments(); AttachIdx++)
    {
        CScriptAttachNode *pAttach = pParent->Attachment(AttachIdx);

        if (pAttach->LocatorName() == "L_back_claw" || pAttach->LocatorName() == "R_back_claw")
            pAttach->SetRotation(CVector3f(0,0,180));
    }

    // Get pincers scale
    mPincersScale = CFloatRef(pInstance, pInstance->Template()->Properties()->ChildByID(0x3DB583AE));
    if (mPincersScale.IsValid()) PropertyModified(mPincersScale.Property());
}

void CSandwormExtra::PropertyModified(IPropertyNew* pProp)
{
    if (pProp == mPincersScale)
    {
        for (u32 AttachIdx = 0; AttachIdx < mpScriptNode->NumAttachments(); AttachIdx++)
        {
            CScriptAttachNode* pAttach = mpScriptNode->Attachment(AttachIdx);
            pAttach->SetScale( CVector3f(mPincersScale) );
        }
    }
}
