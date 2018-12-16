#include "CAddLinkCommand.h"
#include <Core/Resource/Script/CLink.h>

CAddLinkCommand::CAddLinkCommand(CWorldEditor *pEditor, CLink Link)
    : IUndoCommand("Add Link")
    , mpEditor(pEditor)
    , mLink(Link)
{
    mAffectedInstances << mLink.Sender();

    if (mLink.SenderID() != mLink.ReceiverID())
        mAffectedInstances << mLink.Receiver();
}

void CAddLinkCommand::undo()
{
    CScriptObject *pSender = mLink.Sender();
    CScriptObject *pReceiver = mLink.Receiver();
    uint32 SenderIndex = pSender->NumLinks(ELinkType::Outgoing) - 1;
    CLink *pLink = pSender->Link(ELinkType::Outgoing, SenderIndex);
    pSender->RemoveLink(ELinkType::Outgoing, pLink);
    pReceiver->RemoveLink(ELinkType::Incoming, pLink);
    delete pLink;

    mpEditor->OnLinksModified(mAffectedInstances.DereferenceList());
}

void CAddLinkCommand::redo()
{
    CLink *pLink = new CLink(mLink);
    pLink->Sender()->AddLink(ELinkType::Outgoing, pLink, -1);
    pLink->Receiver()->AddLink(ELinkType::Incoming, pLink, -1);

    mpEditor->OnLinksModified(mAffectedInstances.DereferenceList());
}
