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

    CPropertyStruct* LoadStructMP1(CInputStream& SCLY, CStructTemplate *tmp);
    CScriptObject* LoadObjectMP1(CInputStream& SCLY);
    CScriptLayer* LoadLayerMP1(CInputStream& SCLY);

    void LoadStructMP2(CInputStream& SCLY, CPropertyStruct *pStruct, CStructTemplate *pTemp);
    CScriptObject* LoadObjectMP2(CInputStream& SCLY);
    CScriptLayer* LoadLayerMP2(CInputStream& SCLY);

    void SetupAttribs();

public:
    static CScriptLayer* LoadLayer(CInputStream& SCLY, CGameArea *pArea, EGame version);
};

#endif // CSCRIPTLOADER_H
