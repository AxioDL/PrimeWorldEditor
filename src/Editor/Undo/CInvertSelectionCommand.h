#ifndef CINVERTSELECTIONCOMMAND_H
#define CINVERTSELECTIONCOMMAND_H

#include "IUndoCommand.h"
#include "Editor/INodeEditor.h"
#include <Core/Scene/CSceneNode.h>

class CInvertSelectionCommand : public IUndoCommand
{
    CNodeSelection *mpSelection;
    QList<CSceneNode*> mOldSelection;
    QList<CSceneNode*> mNewSelection;

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

    void undo() { mpSelection->SetSelectedNodes(mOldSelection); }
    void redo() { mpSelection->SetSelectedNodes(mNewSelection); }
    bool AffectsCleanState() const { return false; }
};

#endif // CINVERTSELECTIONCOMMAND_H
