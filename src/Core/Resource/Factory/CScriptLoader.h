#ifndef CSCRIPTLOADER_H
#define CSCRIPTLOADER_H

#include "Core/Resource/Script/CScriptObject.h"
#include "Core/Resource/Script/CScriptLayer.h"
#include "Core/Resource/Script/CMasterTemplate.h"
#include "Core/Resource/CGameArea.h"
#include "Core/Resource/CResCache.h"

class CScriptLoader
{
    EGame mVersion;
    CScriptObject *mpObj;
    CScriptLayer *mpLayer;
    CGameArea *mpArea;
    CMasterTemplate *mpMaster;

    CScriptLoader();
    void ReadProperty(IProperty *pProp, u32 Size, IInputStream& rSCLY);

    void LoadStructMP1(IInputStream& rSCLY, CPropertyStruct *pStruct, CStructTemplate *pTemp);
    CScriptObject* LoadObjectMP1(IInputStream& rSCLY);
    CScriptLayer* LoadLayerMP1(IInputStream& rSCLY);

    void LoadStructMP2(IInputStream& rSCLY, CPropertyStruct *pStruct, CStructTemplate *pTemp);
    CScriptObject* LoadObjectMP2(IInputStream& rSCLY);
    CScriptLayer* LoadLayerMP2(IInputStream& rSCLY);

public:
    static CScriptLayer* LoadLayer(IInputStream& rSCLY, CGameArea *pArea, EGame Version);
    static CScriptObject* LoadInstance(IInputStream& rSCLY, CGameArea *pArea, CScriptLayer *pLayer, EGame Version, bool ForceReturnsFormat);
};

#endif // CSCRIPTLOADER_H
