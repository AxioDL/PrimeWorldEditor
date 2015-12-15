#include "CSelectNodeCommand.h"
#include "Editor/INodeEditor.h"

CSelectNodeCommand::CSelectNodeCommand(INodeEditor *pEditor, CSceneNode *pNode, QList<CSceneNode*>& selection)
    : QUndoCommand("Select"),
      mpEditor(pEditor),
      mpNode(pNode),
      mpSelection(&selection)
{
}

void CSelectNodeCommand::undo()
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

void CSelectNodeCommand::redo()
{
    if (!mpNode->IsSelected())
    {
        mpNode->SetSelected(true);
        mpSelection->push_back(mpNode);
    }

    mpEditor->ExpandSelectionBounds(mpNode);
    mpEditor->SelectionModified();
}
