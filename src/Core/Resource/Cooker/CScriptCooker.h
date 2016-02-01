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
    void WriteProperty(IProperty *pProp);
    void WriteLayerMP1(CScriptLayer *pLayer);
    void WriteInstanceMP1(CScriptObject *pInstance);

public:
    static void WriteLayer(EGame Game, CScriptLayer *pLayer, IOutputStream& rOut);
};

#endif // CSCRIPTCOOKER_H
