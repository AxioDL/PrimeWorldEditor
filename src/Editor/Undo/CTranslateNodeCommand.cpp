#include "CTranslateNodeCommand.h"
#include "EUndoCommand.h"
#include "Editor/INodeEditor.h"

CTranslateNodeCommand::CTranslateNodeCommand()
    : QUndoCommand("Translate"),
      mpEditor(nullptr),
      mCommandEnded(false)
{
}

CTranslateNodeCommand::CTranslateNodeCommand(INodeEditor *pEditor, const QList<CSceneNode*>& nodes, const CVector3f& delta, ETransformSpace transformSpace)
    : QUndoCommand("Translate"),
      mpEditor(pEditor),
      mCommandEnded(false)
{
    mNodeList.reserve(nodes.size());

    foreach (CSceneNode *pNode, nodes)
    {
        SNodeTranslate translate;
        translate.pNode = pNode;
        translate.initialPos = pNode->LocalPosition();
        pNode->Translate(delta, transformSpace);
        translate.newPos = pNode->LocalPosition();
        mNodeList.push_back(translate);
    }

    mpEditor->SelectionTransformed();
}

CTranslateNodeCommand::~CTranslateNodeCommand()
{
}

int CTranslateNodeCommand::id() const
{
    return eTranslateNodeCmd;
}

bool CTranslateNodeCommand::mergeWith(const QUndoCommand *other)
{
    if (mCommandEnded) return false;

    if (other->id() == eTranslateNodeCmd)
    {
        const CTranslateNodeCommand *pCmd = static_cast<const CTranslateNodeCommand*>(other);

        if (pCmd->mCommandEnded)
        {
            mCommandEnded = true;
            return true;
        }

        if ((mpEditor == pCmd->mpEditor) && (mNodeList.size() == pCmd->mNodeList.size()))
        {
            for (int iNode = 0; iNode < mNodeList.size(); iNode++)
                mNodeList[iNode].newPos = pCmd->mNodeList[iNode].newPos;

            return true;
        }
    }

    return false;
}

void CTranslateNodeCommand::undo()
{
    if (!mpEditor) return;

    foreach (SNodeTranslate translate, mNodeList)
        translate.pNode->SetPosition(translate.initialPos);

    mpEditor->RecalculateSelectionBounds();
    mpEditor->SelectionTransformed();
    mpEditor->UpdateGizmoUI();
}

void CTranslateNodeCommand::redo()
{
    if (!mpEditor) return;

    foreach (SNodeTranslate translate, mNodeList)
        translate.pNode->SetPosition(translate.newPos);

    mpEditor->RecalculateSelectionBounds();
    mpEditor->SelectionTransformed();
    mpEditor->UpdateGizmoUI();
}

CTranslateNodeCommand* CTranslateNodeCommand::End()
{
    CTranslateNodeCommand *pCmd = new CTranslateNodeCommand();
    pCmd->mCommandEnded = true;
    return pCmd;
}
