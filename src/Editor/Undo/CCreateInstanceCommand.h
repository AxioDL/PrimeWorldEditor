#ifndef CCREATEINSTANCECOMMAND_H
#define CCREATEINSTANCECOMMAND_H

#include "IUndoCommand.h"
#include "CClearSelectionCommand.h"
#include "CSelectNodeCommand.h"
#include "Editor/WorldEditor/CWorldEditor.h"
#include <Core/Resource/CGameArea.h>
#include <Core/Resource/Script/CScriptLayer.h>
#include <Core/Resource/Script/CScriptObject.h>

class CCreateInstanceCommand : public IUndoCommand
{
    CWorldEditor *mpEditor;
    CScene *mpScene;
    CGameArea *mpArea;
    CScriptTemplate *mpTemplate;
    u32 mLayerIndex;
    CVector3f mSpawnPosition;

    QList<CSceneNode*> mOldSelection;
    CScriptObject *mpNewInstance;
    CScriptNode *mpNewNode;

public:
    CCreateInstanceCommand(CWorldEditor *pEditor, CScriptTemplate *pTemplate, CScriptLayer *pLayer, const CVector3f& rkPosition);
    void undo();
    void redo();
    bool AffectsCleanState() const { return true; }
};

#endif // CCREATEINSTANCECOMMAND_H
