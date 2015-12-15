#ifndef CDESELECTNODECOMMAND_H
#define CDESELECTNODECOMMAND_H

#include <QUndoCommand>
#include "../INodeEditor.h"
#include <Scene/CSceneNode.h>

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
