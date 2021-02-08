#ifndef CCLEARSELECTIONCOMMAND_H
#define CCLEARSELECTIONCOMMAND_H

#include "IUndoCommand.h"
#include "ObjReferences.h"
#include "Editor/CSelectionIterator.h"
#include "Editor/INodeEditor.h"
#include <Core/Scene/CSceneNode.h>

class CClearSelectionCommand : public IUndoCommand
{
    CNodeSelection *mpSelection;
    CNodePtrList mOldSelection;

public:
    explicit CClearSelectionCommand(CNodeSelection *pSelection)
        : IUndoCommand("Clear Selection"),
          mpSelection(pSelection)
    {
        for (CSelectionIterator It(pSelection); It; ++It)
            mOldSelection.push_back(*It);
    }

    void undo() override { mpSelection->SetSelectedNodes(mOldSelection.DereferenceList()); }
    void redo() override { mpSelection->Clear(); }
    bool AffectsCleanState() const override { return false; }
};

#endif // CCLEARSELECTIONCOMMAND_H
