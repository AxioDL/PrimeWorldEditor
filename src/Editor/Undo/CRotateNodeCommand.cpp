#include "CRotateNodeCommand.h"
#include "EUndoCommand.h"
#include "Editor/INodeEditor.h"

CRotateNodeCommand::CRotateNodeCommand()
    : QUndoCommand("Rotate"),
      mpEditor(nullptr),
      mCommandEnded(false)
{
}

CRotateNodeCommand::CRotateNodeCommand(INodeEditor *pEditor, const QList<CSceneNode*>& nodes, const CVector3f& /*pivot*/, const CQuaternion& delta, ETransformSpace transformSpace)
    : QUndoCommand("Rotate"),
      mpEditor(pEditor),
      mCommandEnded(false)
{
    mNodeList.reserve(nodes.size());

    foreach (CSceneNode *pNode, nodes)
    {
        SNodeRotate rotate;
        rotate.pNode = pNode;
        rotate.initialPos = pNode->LocalPosition();
        rotate.initialRot = pNode->LocalRotation();
        pNode->Rotate(delta, transformSpace);
        rotate.newPos = pNode->LocalPosition();
        rotate.newRot = pNode->LocalRotation();
        mNodeList.push_back(rotate);
    }

    mpEditor->NotifySelectionTransformed();
}

CRotateNodeCommand::~CRotateNodeCommand()
{
}

int CRotateNodeCommand::id() const
{
    return eRotateNodeCmd;
}

bool CRotateNodeCommand::mergeWith(const QUndoCommand *other)
{
    if (mCommandEnded) return false;

    if (other->id() == eRotateNodeCmd)
    {
        const CRotateNodeCommand *pCmd = static_cast<const CRotateNodeCommand*>(other);

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
                mNodeList[iNode].newRot = pCmd->mNodeList[iNode].newRot;
            }

            return true;
        }
    }

    return false;
}

void CRotateNodeCommand::undo()
{
    if (!mpEditor) return;

    foreach (SNodeRotate rotate, mNodeList)
    {
        rotate.pNode->SetPosition(rotate.initialPos);
        rotate.pNode->SetRotation(rotate.initialRot);
    }

    mpEditor->RecalculateSelectionBounds();
    mpEditor->NotifySelectionTransformed();
    mpEditor->UpdateGizmoUI();
}

void CRotateNodeCommand::redo()
{
    if (!mpEditor) return;

    foreach (SNodeRotate rotate, mNodeList)
    {
        rotate.pNode->SetPosition(rotate.newPos);
        rotate.pNode->SetRotation(rotate.newRot);
    }

    mpEditor->RecalculateSelectionBounds();
    mpEditor->NotifySelectionTransformed();
    mpEditor->UpdateGizmoUI();
}

CRotateNodeCommand* CRotateNodeCommand::End()
{
    CRotateNodeCommand *pCmd = new CRotateNodeCommand();
    pCmd->mCommandEnded = true;
    return pCmd;
}
