#include "Editor/Undo/CAddLinkCommand.h"

#include "Editor/WorldEditor/CWorldEditor.h"
#include <Core/Resource/Script/CLink.h>

#include <QCoreApplication>

CAddLinkCommand::CAddLinkCommand(CWorldEditor *pEditor, const CLink& Link)
    : IUndoCommand(QCoreApplication::translate("CAddLinkCommand", "Add Link"))
    , mpEditor(pEditor)
    , mLink(Link)
{
    mAffectedInstances.push_back(mLink.Sender());

    if (mLink.SenderID() != mLink.ReceiverID())
        mAffectedInstances.push_back(mLink.Receiver());
}

void CAddLinkCommand::undo()
{
    CScriptObject *pSender = mLink.Sender();
    CScriptObject *pReceiver = mLink.Receiver();
    const size_t SenderIndex = pSender->NumLinks(ELinkType::Outgoing) - 1;
    CLink *pLink = pSender->Link(ELinkType::Outgoing, SenderIndex);
    pSender->RemoveLink(ELinkType::Outgoing, pLink);
    pReceiver->RemoveLink(ELinkType::Incoming, pLink);
    delete pLink;

    mpEditor->OnLinksModified(mAffectedInstances.DereferenceList());
}

void CAddLinkCommand::redo()
{
    CLink *pLink = new CLink(mLink);
    pLink->Sender()->AddLink(ELinkType::Outgoing, pLink);
    pLink->Receiver()->AddLink(ELinkType::Incoming, pLink);

    mpEditor->OnLinksModified(mAffectedInstances.DereferenceList());
}
