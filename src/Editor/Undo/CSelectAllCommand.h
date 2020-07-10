#ifndef CSELECTALLCOMMAND_H
#define CSELECTALLCOMMAND_H

#include "IUndoCommand.h"
#include "ObjReferences.h"
#include "Editor/CSelectionIterator.h"
#include "Editor/INodeEditor.h"
#include <Core/Scene/CSceneNode.h>

class CSelectAllCommand : public IUndoCommand
{
    CNodePtrList mOldSelection;
    CNodePtrList mNewSelection;
    CNodeSelection *mpSelection;

public:
    CSelectAllCommand(CNodeSelection *pSelection, CScene *pScene, FNodeFlags NodeFlags)
        : IUndoCommand("Select All")
        , mpSelection(pSelection)
    {
        for (CSelectionIterator It(pSelection); It; ++It)
            mOldSelection.push_back(*It);
        for (CSceneIterator It(pScene, NodeFlags); It; ++It)
            mNewSelection.push_back(*It);
    }

    void undo() override { mpSelection->SetSelectedNodes(mOldSelection.DereferenceList()); }
    void redo() override { mpSelection->SetSelectedNodes(mNewSelection.DereferenceList()); }
    bool AffectsCleanState() const override { return false; }
};

#endif // CSELECTALLCOMMAND_H
