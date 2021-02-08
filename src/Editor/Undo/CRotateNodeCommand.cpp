#include "CRotateNodeCommand.h"
#include "EUndoCommand.h"
#include "Editor/INodeEditor.h"

CRotateNodeCommand::CRotateNodeCommand()
    : IUndoCommand("Rotate")
{
}

CRotateNodeCommand::CRotateNodeCommand(
        INodeEditor *pEditor,
        const QList<CSceneNode*>& rkNodes,
        bool UsePivot,
        const CVector3f& rkPivot,
        const CQuaternion& rkPivotRotation,
        const CQuaternion& rkDelta,
        ETransformSpace TransformSpace
    )
    : IUndoCommand("Rotate"),
      mpEditor(pEditor)
{
    mNodeList.reserve(rkNodes.size());

    for (CSceneNode *pNode : rkNodes)
    {
        SNodeRotate Rotate;
        Rotate.pNode = pNode;
        Rotate.InitialPos = pNode->LocalPosition();
        Rotate.InitialRot = pNode->LocalRotation();

        if (UsePivot)
            pNode->Rotate(rkDelta, rkPivot, rkPivotRotation, TransformSpace);
        else
            pNode->Rotate(rkDelta, TransformSpace);

        Rotate.NewPos = pNode->LocalPosition();
        Rotate.NewRot = pNode->LocalRotation();
        mNodeList.push_back(Rotate);
    }

    mpEditor->NotifySelectionTransformed();
}

CRotateNodeCommand::~CRotateNodeCommand() = default;

int CRotateNodeCommand::id() const
{
    return (int) EUndoCommand::RotateNodeCmd;
}

bool CRotateNodeCommand::mergeWith(const QUndoCommand *pkOther)
{
    if (mCommandEnded) return false;

    if (pkOther->id() == (int) EUndoCommand::RotateNodeCmd)
    {
        const CRotateNodeCommand *pkCmd = static_cast<const CRotateNodeCommand*>(pkOther);

        if (pkCmd->mCommandEnded)
        {
            mCommandEnded = true;
            return true;
        }

        if ((mpEditor == pkCmd->mpEditor) && (mNodeList.size() == pkCmd->mNodeList.size()))
        {
            for (int iNode = 0; iNode < mNodeList.size(); iNode++)
            {
                mNodeList[iNode].NewPos = pkCmd->mNodeList[iNode].NewPos;
                mNodeList[iNode].NewRot = pkCmd->mNodeList[iNode].NewRot;
            }

            return true;
        }
    }

    return false;
}

void CRotateNodeCommand::undo()
{
    if (!mpEditor) return;

    for (SNodeRotate& Rotate : mNodeList)
    {
        Rotate.pNode->SetPosition(Rotate.InitialPos);
        Rotate.pNode->SetRotation(Rotate.InitialRot);
    }

    mpEditor->NotifySelectionTransformed();
    mpEditor->UpdateGizmoUI();
}

void CRotateNodeCommand::redo()
{
    if (!mpEditor) return;

    for (SNodeRotate& Rotate : mNodeList)
    {
        Rotate.pNode->SetPosition(Rotate.NewPos);
        Rotate.pNode->SetRotation(Rotate.NewRot);
    }

    mpEditor->NotifySelectionTransformed();
    mpEditor->UpdateGizmoUI();
}

CRotateNodeCommand* CRotateNodeCommand::End()
{
    CRotateNodeCommand *pCmd = new CRotateNodeCommand();
    pCmd->mCommandEnded = true;
    return pCmd;
}
