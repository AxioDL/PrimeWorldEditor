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
    void ReadProperty(IProperty *pProp, u32 Size, IInputStream& SCLY);

    void LoadStructMP1(IInputStream& SCLY, CPropertyStruct *pStruct, CStructTemplate *pTemp);
    CScriptObject* LoadObjectMP1(IInputStream& SCLY);
    CScriptLayer* LoadLayerMP1(IInputStream& SCLY);

    void LoadStructMP2(IInputStream& SCLY, CPropertyStruct *pStruct, CStructTemplate *pTemp);
    CScriptObject* LoadObjectMP2(IInputStream& SCLY);
    CScriptLayer* LoadLayerMP2(IInputStream& SCLY);

    void SetupAttribs();

public:
    static CScriptLayer* LoadLayer(IInputStream& SCLY, CGameArea *pArea, EGame version);
};

#endif // CSCRIPTLOADER_H
