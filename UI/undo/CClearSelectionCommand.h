#ifndef CCLEARSELECTIONCOMMAND_H
#define CCLEARSELECTIONCOMMAND_H

#include <QUndoCommand>
#include "../INodeEditor.h"
#include <Scene/CSceneNode.h>

class CClearSelectionCommand : public QUndoCommand
{
    INodeEditor *mpEditor;
    QList<CSceneNode*> mSelectionState;
    QList<CSceneNode*> *mpSelection;
public:
    CClearSelectionCommand(INodeEditor *pEditor, QList<CSceneNode*>& selection);
    ~CClearSelectionCommand();
    void undo();
    void redo();
};

#endif // CCLEARSELECTIONCOMMAND_H
