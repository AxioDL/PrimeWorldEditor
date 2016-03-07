#include "CAddLinkCommand.h"
#include <Core/Resource/Script/CLink.h>

CAddLinkCommand::CAddLinkCommand(CWorldEditor *pEditor, CLink Link)
    : IUndoCommand("Add Link")
    , mpEditor(pEditor)
    , mLink(Link)
{
    mAffectedInstances << mLink.Sender();

    if (mLink.Sender() != mLink.Receiver())
        mAffectedInstances << mLink.Receiver();
}

void CAddLinkCommand::undo()
{
    CScriptObject *pSender = mLink.Sender();
    CScriptObject *pReceiver = mLink.Receiver();
    u32 SenderIndex = pSender->NumLinks(eOutgoing) - 1;
    CLink *pLink = pSender->Link(eOutgoing, SenderIndex);
    pSender->RemoveLink(eOutgoing, pLink);
    pReceiver->RemoveLink(eIncoming, pLink);
    delete pLink;

    mpEditor->InstanceLinksModified(mAffectedInstances);
}

void CAddLinkCommand::redo()
{
    CLink *pLink = new CLink(mLink);
    pLink->Sender()->AddLink(eOutgoing, pLink, -1);
    pLink->Receiver()->AddLink(eIncoming, pLink, -1);

    mpEditor->InstanceLinksModified(mAffectedInstances);
}
