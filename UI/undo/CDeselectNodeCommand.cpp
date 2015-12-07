#include "CDeselectNodeCommand.h"
#include "../CWorldEditor.h"

CDeselectNodeCommand::CDeselectNodeCommand(INodeEditor *pEditor, CSceneNode *pNode, QList<CSceneNode*>& selection)
    : QUndoCommand("Deselect"),
      mpEditor(pEditor),
      mpNode(pNode),
      mpSelection(&selection)
{
}

void CDeselectNodeCommand::undo()
{
    if (!mpNode->IsSelected())
    {
        mpNode->SetSelected(true);
        mpSelection->push_back(mpNode);
    }

    mpEditor->ExpandSelectionBounds(mpNode);
    mpEditor->SelectionModified();
}

void CDeselectNodeCommand::redo()
{
    if (mpNode->IsSelected())
    {
        mpNode->SetSelected(false);

        for (auto it = mpSelection->begin(); it != mpSelection->end(); it++)
        {
            if (*it == mpNode)
            {
                mpSelection->erase(it);
                break;
            }
        }
    }

    mpEditor->RecalculateSelectionBounds();
    mpEditor->SelectionModified();
}
