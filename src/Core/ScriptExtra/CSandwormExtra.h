#ifndef CSANDWORMEXTRA_H
#define CSANDWORMEXTRA_H

#include "CScriptExtra.h"

class CSandwormExtra : public CScriptExtra
{
    // Transform adjustments to Sandworm attachments.
    CFloatRef mPincersScale;

public:
    explicit CSandwormExtra(CScriptObject* pInstance, CScene* pScene, CScriptNode* pParent);
    void PropertyModified(IProperty* pProp) override;
};

#endif // CSANDWORMEXTRA_H
