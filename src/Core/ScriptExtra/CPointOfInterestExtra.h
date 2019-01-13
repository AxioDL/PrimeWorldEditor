#ifndef CPOINTOFINTERESTEXTRA_H
#define CPOINTOFINTERESTEXTRA_H

#include "CScriptExtra.h"
#include "Core/Resource/Scan/CScan.h"
#include <Common/CColor.h>

class CPointOfInterestExtra : public CScriptExtra
{
    // Tint POI billboard orange/red depending on scan importance
    CAssetRef mScanProperty;
    TResPtr<CScan> mpScanData;
    CBoolRef mScanIsCritical;

public:
    explicit CPointOfInterestExtra(CScriptObject *pInstance, CScene *pScene, CScriptNode *pParent = 0);
    void PropertyModified(IProperty* pProperty);
    void ModifyTintColor(CColor& Color);
    CScan* GetScan() const { return mpScanData; }

    static const CColor skRegularColor;
    static const CColor skImportantColor;
};

#endif // CPOINTOFINTERESTEXTRA_H
