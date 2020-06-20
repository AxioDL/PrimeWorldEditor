#ifndef CSCRIPTLOADER_H
#define CSCRIPTLOADER_H

#include "Core/GameProject/CResourceStore.h"
#include "Core/Resource/Area/CGameArea.h"
#include "Core/Resource/Script/CScriptObject.h"
#include "Core/Resource/Script/CScriptLayer.h"
#include "Core/Resource/Script/CGameTemplate.h"
#include <memory>

class CScriptLoader
{
    EGame mVersion{};
    CScriptObject* mpObj = nullptr;
    CScriptLayer* mpLayer = nullptr;
    CGameArea* mpArea = nullptr;
    CGameTemplate *mpGameTemplate = nullptr;

    // Current data pointer
    void* mpCurrentData = nullptr;

    CScriptLoader();
    void ReadProperty(IProperty* pProp, uint32 Size, IInputStream& rSCLY);

    void LoadStructMP1(IInputStream& rSCLY, CStructProperty* pStruct);
    CScriptObject* LoadObjectMP1(IInputStream& rSCLY);
    std::unique_ptr<CScriptLayer> LoadLayerMP1(IInputStream& rSCLY);

    void LoadStructMP2(IInputStream& rSCLY, CStructProperty* pStruct);
    CScriptObject* LoadObjectMP2(IInputStream& rSCLY);
    std::unique_ptr<CScriptLayer> LoadLayerMP2(IInputStream& rSCLY);

public:
    static std::unique_ptr<CScriptLayer> LoadLayer(IInputStream& rSCLY, CGameArea *pArea, EGame Version);
    static CScriptObject* LoadInstance(IInputStream& rSCLY, CGameArea *pArea, CScriptLayer *pLayer, EGame Version, bool ForceReturnsFormat);
    static void LoadStructData(IInputStream& rInput, CStructRef InStruct);
};

#endif // CSCRIPTLOADER_H
