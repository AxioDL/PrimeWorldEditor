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
                mOldSelection.push_back(*It);
            else
                mNewSelection.push_back(*It);
        }
    }

    void undo() override { mpSelection->SetSelectedNodes(mOldSelection.DereferenceList()); }
    void redo() override { mpSelection->SetSelectedNodes(mNewSelection.DereferenceList()); }
    bool AffectsCleanState() const override { return false; }
};

#endif // CINVERTSELECTIONCOMMAND_H
