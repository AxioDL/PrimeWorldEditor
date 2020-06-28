#include "CScaleNodeCommand.h"
#include "EUndoCommand.h"
#include "Editor/INodeEditor.h"

CScaleNodeCommand::CScaleNodeCommand()
    : IUndoCommand("Scale")
{
}

CScaleNodeCommand::CScaleNodeCommand(INodeEditor *pEditor, const QList<CSceneNode*>& rkNodes, bool UsePivot, const CVector3f& rkPivot, const CVector3f& rkDelta)
    : IUndoCommand("Scale"),
      mpEditor(pEditor)
{
    mNodeList.reserve(rkNodes.size());

    for (CSceneNode *pNode : rkNodes)
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

CScaleNodeCommand::~CScaleNodeCommand()
{
}

int CScaleNodeCommand::id() const
{
    return (int) EUndoCommand::ScaleNodeCmd;
}

bool CScaleNodeCommand::mergeWith(const QUndoCommand *pkOther)
{
    if (mCommandEnded) return false;

    if (pkOther->id() == (int) EUndoCommand::ScaleNodeCmd)
    {
        const CScaleNodeCommand *pkCmd = static_cast<const CScaleNodeCommand*>(pkOther);

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
    CScaleNodeCommand *pCmd = new CScaleNodeCommand();
    pCmd->mCommandEnded = true;
    return pCmd;
}
