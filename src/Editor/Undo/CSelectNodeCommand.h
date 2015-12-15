#ifndef CSELECTNODECOMMAND_H
#define CSELECTNODECOMMAND_H

#include <QUndoCommand>
#include "../INodeEditor.h"
#include <Scene/CSceneNode.h>

class CSelectNodeCommand : public QUndoCommand
{
    INodeEditor *mpEditor;
    CSceneNode *mpNode;
    QList<CSceneNode*> *mpSelection;
public:
    CSelectNodeCommand(INodeEditor *pEditor, CSceneNode *pNode, QList<CSceneNode*>& selection);
    void undo();
    void redo();
};

#endif // CSELECTNODECOMMAND_H
