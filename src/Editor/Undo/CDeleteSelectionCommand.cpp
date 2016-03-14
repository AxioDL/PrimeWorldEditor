#include "CDeleteSelectionCommand.h"
#include "Editor/CSelectionIterator.h"

CDeleteSelectionCommand::CDeleteSelectionCommand(CWorldEditor *pEditor)
    : IUndoCommand("Delete")
    , mpEditor(pEditor)
{
    mOldSelection = pEditor->Selection()->SelectedNodeList();

    for (CSelectionIterator It(pEditor->Selection()); It; ++It)
    {
        if (It->NodeType() == eScriptNode)
            mNodesToDelete << *It;
        else
            mNewSelection << *It;
    }
}

void CDeleteSelectionCommand::undo()
{
    //foreach (CSceneNode *pNode, mNodesToDelete)
    //    pNode->SetDeleted(false);
    //mpEditor->Selection()->SetSelectedNodes(mOldSelection);
}

void CDeleteSelectionCommand::redo()
{
    mpEditor->Selection()->SetSelectedNodes(mNewSelection);

    foreach (CSceneNode *pNode, mNodesToDelete)
    {
        CScriptObject *pInst = static_cast<CScriptNode*>(pNode)->Object();
        mpEditor->NotifyNodeAboutToBeDeleted(pNode);
        mpEditor->Scene()->DeleteNode(pNode);
        mpEditor->ActiveArea()->DeleteInstance(pInst);
        mpEditor->NotifyNodeDeleted();
    }
}
