#include "Editor/Undo/CRotateNodeCommand.h"

#include "Editor/INodeEditor.h"
#include "Editor/Undo/EUndoCommand.h"
#include <Core/Scene/CSceneNode.h>

#include <QCoreApplication>

struct CRotateNodeCommand::SNodeRotate
{
    CNodePtr pNode;
    CVector3f InitialPos;
    CQuaternion InitialRot;
    CVector3f NewPos;
    CQuaternion NewRot;
};

CRotateNodeCommand::CRotateNodeCommand()
    : IUndoCommand(QCoreApplication::translate("CRotateNodeCommand", "Rotate"))
{
}

CRotateNodeCommand::CRotateNodeCommand(
        INodeEditor *pEditor,
        std::span<CSceneNode*> nodes,
        bool UsePivot,
        const CVector3f& rkPivot,
        const CQuaternion& rkPivotRotation,
        const CQuaternion& rkDelta,
        ETransformSpace TransformSpace
    )
    : IUndoCommand(QCoreApplication::translate("CRotateNodeCommand", "Rotate")),
      mpEditor(pEditor)
{
    mNodeList.reserve(std::ssize(nodes));

    for (CSceneNode *pNode : nodes)
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
    return int(EUndoCommand::RotateNodeCmd);
}

bool CRotateNodeCommand::mergeWith(const QUndoCommand *pkOther)
{
    if (mCommandEnded)
        return false;

    if (pkOther->id() == int(EUndoCommand::RotateNodeCmd))
    {
        const auto* pkCmd = static_cast<const CRotateNodeCommand*>(pkOther);

        if (pkCmd->mCommandEnded)
        {
            mCommandEnded = true;
            return true;
        }

        if ((mpEditor == pkCmd->mpEditor) && (mNodeList.size() == pkCmd->mNodeList.size()))
        {
            for (qsizetype iNode = 0; iNode < mNodeList.size(); iNode++)
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
    if (!mpEditor)
        return;

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
    if (!mpEditor)
        return;

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
    auto* pCmd = new CRotateNodeCommand();
    pCmd->mCommandEnded = true;
    return pCmd;
}
