#ifndef CCLEARSELECTIONCOMMAND_H
#define CCLEARSELECTIONCOMMAND_H

#include "IUndoCommand.h"
#include "Editor/INodeEditor.h"
#include <Core/Scene/CSceneNode.h>

class CClearSelectionCommand : public IUndoCommand
{
    QList<CSceneNode*> mOldSelection;
    CNodeSelection *mpSelection;

public:
    CClearSelectionCommand(CNodeSelection *pSelection)
        : IUndoCommand("Clear Selection"),
          mOldSelection(pSelection->SelectedNodeList()),
          mpSelection(pSelection)
    {}

    void undo() { mpSelection->SetSelectedNodes(mOldSelection); }
    void redo() { mpSelection->Clear(); }
    bool AffectsCleanState() const { return false; }
};

#endif // CCLEARSELECTIONCOMMAND_H
