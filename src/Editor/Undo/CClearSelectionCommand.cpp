#include "CClearSelectionCommand.h"
#include "Editor/INodeEditor.h"

CClearSelectionCommand::CClearSelectionCommand(INodeEditor *pEditor, QList<CSceneNode*>& selection)
    : IUndoCommand("Clear Selection"),
      mpEditor(pEditor),
      mSelectionState(selection),
      mpSelection(&selection)
{
}

CClearSelectionCommand::~CClearSelectionCommand()
{
}

void CClearSelectionCommand::undo()
{
    mpSelection->reserve(mSelectionState.size());

    foreach (CSceneNode *pNode, mSelectionState)
    {
        if (!pNode->IsSelected())
        {
            pNode->SetSelected(true);
            mpSelection->push_back(pNode);
        }
    }

    mpEditor->RecalculateSelectionBounds();
    mpEditor->NotifySelectionModified();
}

void CClearSelectionCommand::redo()
{
    foreach (CSceneNode *pNode, *mpSelection)
        if (pNode->IsSelected())
            pNode->SetSelected(false);

    mpSelection->clear();
    mpEditor->RecalculateSelectionBounds();
    mpEditor->NotifySelectionModified();
}
