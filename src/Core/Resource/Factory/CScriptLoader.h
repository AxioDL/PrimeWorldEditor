#ifndef CSCRIPTLOADER_H
#define CSCRIPTLOADER_H

#include "Core/GameProject/CResourceStore.h"
#include "Core/Resource/Area/CGameArea.h"
#include "Core/Resource/Script/CScriptObject.h"
#include "Core/Resource/Script/CScriptLayer.h"
#include "Core/Resource/Script/CGameTemplate.h"

class CScriptLoader
{
    EGame mVersion;
    CScriptObject* mpObj;
    CScriptLayer* mpLayer;
    CGameArea* mpArea;
    CGameTemplate *mpGameTemplate;

    // Current array item pointer
    void* mpArrayItemData;

    CScriptLoader();
    void ReadProperty(IProperty* pProp, u32 Size, IInputStream& rSCLY);

    void LoadStructMP1(IInputStream& rSCLY, CStructProperty* pStruct);
    CScriptObject* LoadObjectMP1(IInputStream& rSCLY);
    CScriptLayer* LoadLayerMP1(IInputStream& rSCLY);

    void LoadStructMP2(IInputStream& rSCLY, CStructProperty* pStruct);
    CScriptObject* LoadObjectMP2(IInputStream& rSCLY);
    CScriptLayer* LoadLayerMP2(IInputStream& rSCLY);

public:
    static CScriptLayer* LoadLayer(IInputStream& rSCLY, CGameArea *pArea, EGame Version);
    static CScriptObject* LoadInstance(IInputStream& rSCLY, CGameArea *pArea, CScriptLayer *pLayer, EGame Version, bool ForceReturnsFormat);
};

#endif // CSCRIPTLOADER_H
