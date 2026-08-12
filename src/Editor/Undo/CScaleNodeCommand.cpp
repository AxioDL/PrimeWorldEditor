#include "Editor/Undo/CScaleNodeCommand.h"

#include "Editor/INodeEditor.h"
#include "Editor/Undo/EUndoCommand.h"
#include <Core/Scene/CSceneNode.h>

#include <QCoreApplication>

struct CScaleNodeCommand::SNodeScale
{
    CNodePtr pNode;
    CVector3f InitialPos;
    CVector3f InitialScale;
    CVector3f NewPos;
    CVector3f NewScale;
};

CScaleNodeCommand::CScaleNodeCommand()
    : IUndoCommand(QCoreApplication::translate("CScaleNodeCommand", "Scale"))
{
}

CScaleNodeCommand::CScaleNodeCommand(INodeEditor *pEditor, std::span<CSceneNode*> nodes, bool UsePivot, const CVector3f& rkPivot, const CVector3f& rkDelta)
    : IUndoCommand(QCoreApplication::translate("CScaleNodeCommand", "Scale")),
      mpEditor(pEditor)
{
    mNodeList.reserve(std::ssize(nodes));

    for (CSceneNode *pNode : nodes)
    {
        SNodeScale Scale;
        Scale.pNode = pNode;
        Scale.InitialPos = pNode->LocalPosition();
        Scale.InitialScale = pNode->LocalScale();

        if (UsePivot)
            pNode->Scale(rkDelta, rkPivot);
        else
            pNode->Scale(rkDelta);

        pNode->Scale(rkDelta);
        Scale.NewPos = pNode->LocalPosition();
        Scale.NewScale = pNode->LocalScale();
        mNodeList.push_back(Scale);
    }

    mpEditor->NotifySelectionTransformed();
}

CScaleNodeCommand::~CScaleNodeCommand() = default;

int CScaleNodeCommand::id() const
{
    return int(EUndoCommand::ScaleNodeCmd);
}

bool CScaleNodeCommand::mergeWith(const QUndoCommand *pkOther)
{
    if (mCommandEnded)
        return false;

    if (pkOther->id() == int(EUndoCommand::ScaleNodeCmd))
    {
        const auto* pkCmd = static_cast<const CScaleNodeCommand*>(pkOther);

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
                mNodeList[iNode].NewScale = pkCmd->mNodeList[iNode].NewScale;
            }

            return true;
        }
    }

    return false;
}

void CScaleNodeCommand::undo()
{
    if (!mpEditor)
        return;

    for (SNodeScale& Scale : mNodeList)
    {
        Scale.pNode->SetPosition(Scale.InitialPos);
        Scale.pNode->SetScale(Scale.InitialScale);
    }

    mpEditor->NotifySelectionTransformed();
    mpEditor->UpdateGizmoUI();
}

void CScaleNodeCommand::redo()
{
    if (!mpEditor)
        return;

    for (SNodeScale& Scale : mNodeList)
    {
        Scale.pNode->SetPosition(Scale.NewPos);
        Scale.pNode->SetScale(Scale.NewScale);
    }

    mpEditor->NotifySelectionTransformed();
    mpEditor->UpdateGizmoUI();
}

CScaleNodeCommand* CScaleNodeCommand::End()
{
    auto* pCmd = new CScaleNodeCommand();
    pCmd->mCommandEnded = true;
    return pCmd;
}
