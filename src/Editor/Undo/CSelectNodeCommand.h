#ifndef CSELECTNODECOMMAND_H
#define CSELECTNODECOMMAND_H

#include "IUndoCommand.h"
#include "Editor/INodeEditor.h"
#include <Core/Scene/CSceneNode.h>

class CSelectNodeCommand : public IUndoCommand
{
    INodeEditor *mpEditor;
    CSceneNode *mpNode;
    QList<CSceneNode*> *mpSelection;
public:
    CSelectNodeCommand(INodeEditor *pEditor, CSceneNode *pNode, QList<CSceneNode*>& selection);
    void undo();
    void redo();
    bool AffectsCleanState() const { return false; }
};

#endif // CSELECTNODECOMMAND_H
