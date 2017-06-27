#ifndef CSCRIPTCOOKER_H
#define CSCRIPTCOOKER_H

#include "CSectionMgrOut.h"
#include "Core/Resource/Script/CScriptLayer.h"
#include "Core/Resource/Script/CScriptObject.h"
#include <Common/EGame.h>
#include <Common/FileIO.h>

class CScriptCooker
{
    EGame mGame;
    std::vector<CScriptObject*> mGeneratedObjects;
    bool mWriteGeneratedSeparately;

    void WriteProperty(IOutputStream& rOut,IProperty *pProp, bool InSingleStruct);

public:
    CScriptCooker(EGame Game, bool WriteGeneratedObjectsSeparately = true)
        : mGame(Game)
        , mWriteGeneratedSeparately(WriteGeneratedObjectsSeparately && mGame >= eEchoesDemo)
    {}

    void WriteInstance(IOutputStream& rOut, CScriptObject *pInstance);
    void WriteLayer(IOutputStream& rOut, CScriptLayer *pLayer);
    void WriteGeneratedLayer(IOutputStream& rOut);
};

#endif // CSCRIPTCOOKER_H
