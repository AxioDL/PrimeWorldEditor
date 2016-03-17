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
    CClearSelectionCommand(CNodeSelection *pSelection)
        : IUndoCommand("Clear Selection"),
          mpSelection(pSelection)
    {
        for (CSelectionIterator It(pSelection); It; ++It)
            mOldSelection << *It;
    }

    void undo() { mpSelection->SetSelectedNodes(mOldSelection.DereferenceList()); }
    void redo() { mpSelection->Clear(); }
    bool AffectsCleanState() const { return false; }
};

#endif // CCLEARSELECTIONCOMMAND_H
