#include "CInvertSelectionCommand.h"
#include <Core/Scene/CSceneIterator.h>

CInvertSelectionCommand::CInvertSelectionCommand(INodeEditor *pEditor, QList<CSceneNode*>& rSelection, CScene *pScene, FNodeFlags NodeFlags)
    : QUndoCommand("Invert Selection")
    , mpEditor(pEditor)
    , mOldSelection(rSelection)
    , mpSelection(&rSelection)
{
    CSceneIterator it(pScene, NodeFlags);

    while (!it.DoneIterating())
    {
        if (!it->IsSelected())
            mNewSelection.append(*it);
        ++it;
    }
}

CInvertSelectionCommand::~CInvertSelectionCommand()
{
}

void CInvertSelectionCommand::undo()
{
    *mpSelection = mOldSelection;

    // Deselect all nodes in new selection
    foreach (CSceneNode *pNode, mNewSelection)
        pNode->SetSelected(false);

    // Select all nodes in the old selection
    foreach (CSceneNode *pNode, mOldSelection)
        pNode->SetSelected(true);

    // Update editor
    mpEditor->RecalculateSelectionBounds();
    mpEditor->SelectionModified();
}

void CInvertSelectionCommand::redo()
{
    *mpSelection = mNewSelection;

    // Deselect all nodes in the old selection
    foreach (CSceneNode *pNode, mOldSelection)
        pNode->SetSelected(false);

    // Select all nodes in the new selection
    foreach (CSceneNode *pNode, mNewSelection)
        pNode->SetSelected(true);

    // Update editor
    mpEditor->RecalculateSelectionBounds();
    mpEditor->SelectionModified();
}
