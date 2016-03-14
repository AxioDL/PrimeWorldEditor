#ifndef CSELECTALLCOMMAND_H
#define CSELECTALLCOMMAND_H

#include "IUndoCommand.h"
#include "Editor/INodeEditor.h"
#include <Core/Scene/CSceneNode.h>

class CSelectAllCommand : public IUndoCommand
{
    QList<CSceneNode*> mOldSelection;
    QList<CSceneNode*> mNewSelection;
    CNodeSelection *mpSelection;

public:
    CSelectAllCommand(CNodeSelection *pSelection, CScene *pScene, FNodeFlags NodeFlags)
        : IUndoCommand("Select All")
        , mOldSelection(pSelection->SelectedNodeList())
        , mpSelection(pSelection)
    {
        for (CSceneIterator It(pScene, NodeFlags); It; ++It)
            mNewSelection << *It;
    }

    void undo() { mpSelection->SetSelectedNodes(mOldSelection); }
    void redo() { mpSelection->SetSelectedNodes(mNewSelection); }
    bool AffectsCleanState() const { return false; }
};

#endif // CSELECTALLCOMMAND_H
