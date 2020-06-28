#ifndef CCREATEINSTANCECOMMAND_H
#define CCREATEINSTANCECOMMAND_H

#include "IUndoCommand.h"
#include "CClearSelectionCommand.h"
#include "CSelectNodeCommand.h"
#include "Editor/WorldEditor/CWorldEditor.h"
#include <Core/Resource/Area/CGameArea.h>
#include <Core/Resource/Script/CScriptLayer.h>
#include <Core/Resource/Script/CScriptObject.h>

class CCreateInstanceCommand : public IUndoCommand
{
    CWorldEditor *mpEditor;
    CScene *mpScene;
    CGameArea *mpArea;
    CScriptTemplate *mpTemplate;
    uint32 mLayerIndex;
    CVector3f mSpawnPosition;

    CNodePtrList mOldSelection;
    CInstancePtr mpNewInstance;
    CNodePtr mpNewNode;

public:
    CCreateInstanceCommand(CWorldEditor *pEditor, CScriptTemplate *pTemplate, CScriptLayer *pLayer, const CVector3f& rkPosition);
    void undo() override;
    void redo() override;
    bool AffectsCleanState() const override { return true; }
};

#endif // CCREATEINSTANCECOMMAND_H
