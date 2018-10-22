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
    CScriptObject* mpObject;
    void* mpArrayItemData;
    std::vector<CScriptObject*> mGeneratedObjects;
    bool mWriteGeneratedSeparately;

    void WriteProperty(IOutputStream& rOut, IProperty* pProperty, bool InAtomicStruct);

public:
    CScriptCooker(EGame Game, bool WriteGeneratedObjectsSeparately = true)
        : mGame(Game)
        , mpObject(nullptr)
        , mpArrayItemData(nullptr)
        , mWriteGeneratedSeparately(WriteGeneratedObjectsSeparately && mGame >= EGame::EchoesDemo)
    {}

    void WriteInstance(IOutputStream& rOut, CScriptObject *pInstance);
    void WriteLayer(IOutputStream& rOut, CScriptLayer *pLayer);
    void WriteGeneratedLayer(IOutputStream& rOut);
};

#endif // CSCRIPTCOOKER_H
