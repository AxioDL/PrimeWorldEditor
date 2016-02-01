#include "CScaleNodeCommand.h"
#include "EUndoCommand.h"
#include "Editor/INodeEditor.h"

CScaleNodeCommand::CScaleNodeCommand()
    : IUndoCommand("Scale"),
      mpEditor(nullptr),
      mCommandEnded(false)
{
}

CScaleNodeCommand::CScaleNodeCommand(INodeEditor *pEditor, const QList<CSceneNode*>& nodes, const CVector3f& /*pivot*/, const CVector3f& delta)
    : IUndoCommand("Scale"),
      mpEditor(pEditor),
      mCommandEnded(false)
{
    mNodeList.reserve(nodes.size());

    foreach (CSceneNode *pNode, nodes)
    {
        SNodeScale scale;
        scale.pNode = pNode;
        scale.initialPos = pNode->LocalPosition();
        scale.initialScale = pNode->LocalScale();
        pNode->Scale(delta);
        scale.newPos = pNode->LocalPosition();
        scale.newScale = pNode->LocalScale();
        mNodeList.push_back(scale);
    }

    mpEditor->NotifySelectionTransformed();
}

CScaleNodeCommand::~CScaleNodeCommand()
{
}

int CScaleNodeCommand::id() const
{
    return eScaleNodeCmd;
}

bool CScaleNodeCommand::mergeWith(const QUndoCommand *other)
{
    if (mCommandEnded) return false;

    if (other->id() == eScaleNodeCmd)
    {
        const CScaleNodeCommand *pCmd = static_cast<const CScaleNodeCommand*>(other);

        if (pCmd->mCommandEnded)
        {
            mCommandEnded = true;
            return true;
        }

        if ((mpEditor == pCmd->mpEditor) && (mNodeList.size() == pCmd->mNodeList.size()))
        {
            for (int iNode = 0; iNode < mNodeList.size(); iNode++)
            {
                mNodeList[iNode].newPos = pCmd->mNodeList[iNode].newPos;
                mNodeList[iNode].newScale = pCmd->mNodeList[iNode].newScale;
            }

            return true;
        }
    }

    return false;
}

void CScaleNodeCommand::undo()
{
    if (!mpEditor) return;

    foreach (SNodeScale scale, mNodeList)
    {
        scale.pNode->SetPosition(scale.initialPos);
        scale.pNode->SetScale(scale.initialScale);
    }

    mpEditor->RecalculateSelectionBounds();
    mpEditor->NotifySelectionTransformed();
    mpEditor->UpdateGizmoUI();
}

void CScaleNodeCommand::redo()
{
    if (!mpEditor) return;

    foreach (SNodeScale scale, mNodeList)
    {
        scale.pNode->SetPosition(scale.newPos);
        scale.pNode->SetScale(scale.newScale);
    }

    mpEditor->RecalculateSelectionBounds();
    mpEditor->NotifySelectionTransformed();
    mpEditor->UpdateGizmoUI();
}

CScaleNodeCommand* CScaleNodeCommand::End()
{
    CScaleNodeCommand *pCmd = new CScaleNodeCommand();
    pCmd->mCommandEnded = true;
    return pCmd;
}
