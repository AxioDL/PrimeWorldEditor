#ifndef CSCRIPTLOADER_H
#define CSCRIPTLOADER_H

#include "../script/CScriptObject.h"
#include "../script/CScriptLayer.h"
#include "../script/CMasterTemplate.h"
#include "../CGameArea.h"
#include <Core/CResCache.h>

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
