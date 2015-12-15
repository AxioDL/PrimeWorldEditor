#ifndef CCLEARSELECTIONCOMMAND_H
#define CCLEARSELECTIONCOMMAND_H

#include "Editor/INodeEditor.h"
#include <Core/Scene/CSceneNode.h>

#include <QUndoCommand>

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
