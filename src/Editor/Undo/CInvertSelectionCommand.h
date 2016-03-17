#ifndef CINVERTSELECTIONCOMMAND_H
#define CINVERTSELECTIONCOMMAND_H

#include "IUndoCommand.h"
#include "ObjReferences.h"
#include "Editor/INodeEditor.h"
#include <Core/Scene/CSceneNode.h>

class CInvertSelectionCommand : public IUndoCommand
{
    CNodeSelection *mpSelection;
    CNodePtrList mOldSelection;
    CNodePtrList mNewSelection;

public:
    CInvertSelectionCommand(CNodeSelection *pSelection, CScene *pScene, FNodeFlags NodeFlags)
        : IUndoCommand("Invert Selection")
        , mpSelection(pSelection)
    {
        for (CSceneIterator It(pScene, NodeFlags); It; ++It)
        {
            if (It->IsSelected())
                mOldSelection << *It;
            else
                mNewSelection << *It;
        }
    }

    void undo() { mpSelection->SetSelectedNodes(mOldSelection.DereferenceList()); }
    void redo() { mpSelection->SetSelectedNodes(mNewSelection.DereferenceList()); }
    bool AffectsCleanState() const { return false; }
};

#endif // CINVERTSELECTIONCOMMAND_H
