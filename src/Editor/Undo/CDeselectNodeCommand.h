#ifndef CDESELECTNODECOMMAND_H
#define CDESELECTNODECOMMAND_H

#include "Editor/INodeEditor.h"
#include <Core/Scene/CSceneNode.h>

#include <QUndoCommand>

class CDeselectNodeCommand : public QUndoCommand
{
    INodeEditor *mpEditor;
    CSceneNode *mpNode;
    QList<CSceneNode*> *mpSelection;
public:
    CDeselectNodeCommand(INodeEditor *pEditor, CSceneNode *pNode, QList<CSceneNode*>& selection);
    void undo();
    void redo();
};

#endif // CDESELECTNODECOMMAND_H
