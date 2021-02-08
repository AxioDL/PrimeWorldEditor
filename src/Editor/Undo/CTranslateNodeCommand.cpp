#include "CTranslateNodeCommand.h"
#include "EUndoCommand.h"
#include "Editor/INodeEditor.h"

CTranslateNodeCommand::CTranslateNodeCommand()
    : IUndoCommand("Translate")
{
}

CTranslateNodeCommand::CTranslateNodeCommand(INodeEditor *pEditor, const QList<CSceneNode*>& rkNodes, const CVector3f& Delta, ETransformSpace TransformSpace)
    : IUndoCommand("Translate"),
      mpEditor(pEditor)
{
    mNodeList.reserve(rkNodes.size());

    for (CSceneNode *pNode : rkNodes)
    {
        SNodeTranslate Translate;
        Translate.pNode = pNode;
        Translate.InitialPos = pNode->LocalPosition();
        pNode->Translate(Delta, TransformSpace);
        Translate.NewPos = pNode->LocalPosition();
        mNodeList.push_back(Translate);
    }

    mpEditor->NotifySelectionTransformed();
}

int CTranslateNodeCommand::id() const
{
    return (int) EUndoCommand::TranslateNodeCmd;
}

bool CTranslateNodeCommand::mergeWith(const QUndoCommand *pkOther)
{
    if (mCommandEnded) return false;

    if (pkOther->id() == (int) EUndoCommand::TranslateNodeCmd)
    {
        const CTranslateNodeCommand *pkCmd = static_cast<const CTranslateNodeCommand*>(pkOther);

        if (pkCmd->mCommandEnded)
        {
            mCommandEnded = true;
            return true;
        }

        if ((mpEditor == pkCmd->mpEditor) && (mNodeList.size() == pkCmd->mNodeList.size()))
        {
            for (int iNode = 0; iNode < mNodeList.size(); iNode++)
                mNodeList[iNode].NewPos = pkCmd->mNodeList[iNode].NewPos;

            return true;
        }
    }

    return false;
}

void CTranslateNodeCommand::undo()
{
    if (!mpEditor)
        return;

    for (SNodeTranslate& Translate : mNodeList)
        Translate.pNode->SetPosition(Translate.InitialPos);

    mpEditor->NotifySelectionTransformed();
    mpEditor->UpdateGizmoUI();
}

void CTranslateNodeCommand::redo()
{
    if (!mpEditor)
        return;

    for (SNodeTranslate& Translate : mNodeList)
        Translate.pNode->SetPosition(Translate.NewPos);

    mpEditor->NotifySelectionTransformed();
    mpEditor->UpdateGizmoUI();
}

CTranslateNodeCommand* CTranslateNodeCommand::End()
{
    CTranslateNodeCommand *pCmd = new CTranslateNodeCommand();
    pCmd->mCommandEnded = true;
    return pCmd;
}
