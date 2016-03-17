#ifndef CSCRIPTCOOKER_H
#define CSCRIPTCOOKER_H

#include "Core/Resource/EGame.h"
#include "Core/Resource/Script/CScriptLayer.h"
#include "Core/Resource/Script/CScriptObject.h"
#include <FileIO/FileIO.h>

class CScriptCooker
{
    IOutputStream *mpSCLY;
    EGame mVersion;

    CScriptCooker() {}
    void WriteProperty(IProperty *pProp, bool InSingleStruct);
    void WriteLayerMP1(CScriptLayer *pLayer);
    void WriteInstanceMP1(CScriptObject *pInstance);
    void WriteLayerMP2(CScriptLayer *pLayer);
    void WriteInstanceMP2(CScriptObject *pInstance);

public:
    static void WriteLayer(EGame Game, CScriptLayer *pLayer, IOutputStream& rOut);
    static void CookInstance(EGame Game, CScriptObject *pInstance, IOutputStream& rOut);
};

#endif // CSCRIPTCOOKER_H
