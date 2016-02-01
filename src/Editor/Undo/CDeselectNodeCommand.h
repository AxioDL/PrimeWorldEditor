#ifndef CDESELECTNODECOMMAND_H
#define CDESELECTNODECOMMAND_H

#include "IUndoCommand.h"
#include "Editor/INodeEditor.h"
#include <Core/Scene/CSceneNode.h>

class CDeselectNodeCommand : public IUndoCommand
{
    INodeEditor *mpEditor;
    CSceneNode *mpNode;
    QList<CSceneNode*> *mpSelection;
public:
    CDeselectNodeCommand(INodeEditor *pEditor, CSceneNode *pNode, QList<CSceneNode*>& selection);
    void undo();
    void redo();
    bool AffectsCleanState() const { return false; }
};

#endif // CDESELECTNODECOMMAND_H
