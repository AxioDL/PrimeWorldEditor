#ifndef CCLEARSELECTIONCOMMAND_H
#define CCLEARSELECTIONCOMMAND_H

#include "IUndoCommand.h"
#include "Editor/INodeEditor.h"
#include <Core/Scene/CSceneNode.h>

class CClearSelectionCommand : public IUndoCommand
{
    INodeEditor *mpEditor;
    QList<CSceneNode*> mSelectionState;
    QList<CSceneNode*> *mpSelection;
public:
    CClearSelectionCommand(INodeEditor *pEditor, QList<CSceneNode*>& selection);
    ~CClearSelectionCommand();
    void undo();
    void redo();
    bool AffectsCleanState() const { return false; }
};

#endif // CCLEARSELECTIONCOMMAND_H
