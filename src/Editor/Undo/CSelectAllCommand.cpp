#include "CSelectAllCommand.h"
#include <Core/Scene/CSceneIterator.h>

CSelectAllCommand::CSelectAllCommand(INodeEditor *pEditor, QList<CSceneNode *> &rSelection, CScene *pScene, FNodeFlags NodeFlags)
    : QUndoCommand("Select All")
    , mpEditor(pEditor)
    , mOldSelection(rSelection)
    , mpSelection(&rSelection)
{
    CSceneIterator it(pScene, NodeFlags);

    while (!it.DoneIterating())
    {
        mNewSelection.append(*it);
        ++it;
    }
}

CSelectAllCommand::~CSelectAllCommand()
{
}


void CSelectAllCommand::undo()
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
    mpEditor->NotifySelectionModified();
}

void CSelectAllCommand::redo()
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
    mpEditor->NotifySelectionModified();
}
