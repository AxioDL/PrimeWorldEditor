#include "Editor/Undo/CTranslateNodeCommand.h"

#include <Core/Scene/CSceneNode.h>
#include "Editor/INodeEditor.h"
#include "Editor/Undo/EUndoCommand.h"

#include <QCoreApplication>

struct CTranslateNodeCommand::SNodeTranslate
{
    CNodePtr pNode;
    CVector3f InitialPos;
    CVector3f NewPos;
};

CTranslateNodeCommand::CTranslateNodeCommand()
    : IUndoCommand(QCoreApplication::translate("CTranslateNodeCommand", "Translate"))
{
}

CTranslateNodeCommand::CTranslateNodeCommand(INodeEditor *pEditor, std::span<CSceneNode*> nodes, const CVector3f& Delta, ETransformSpace TransformSpace)
    : IUndoCommand(QCoreApplication::translate("CTranslateNodeCommand", "Translate")),
      mpEditor(pEditor)
{
    mNodeList.reserve(std::ssize(nodes));

    for (CSceneNode *pNode : nodes)
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

CTranslateNodeCommand::~CTranslateNodeCommand() = default;

int CTranslateNodeCommand::id() const
{
    return int(EUndoCommand::TranslateNodeCmd);
}

bool CTranslateNodeCommand::mergeWith(const QUndoCommand *pkOther)
{
    if (mCommandEnded)
        return false;

    if (pkOther->id() == int(EUndoCommand::TranslateNodeCmd))
    {
        const auto* pkCmd = static_cast<const CTranslateNodeCommand*>(pkOther);

        if (pkCmd->mCommandEnded)
        {
            mCommandEnded = true;
            return true;
        }

        if ((mpEditor == pkCmd->mpEditor) && (mNodeList.size() == pkCmd->mNodeList.size()))
        {
            for (qsizetype iNode = 0; iNode < mNodeList.size(); iNode++)
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
    auto* pCmd = new CTranslateNodeCommand();
    pCmd->mCommandEnded = true;
    return pCmd;
}
