#include "CEditLinkCommand.h"

CEditLinkCommand::CEditLinkCommand(CWorldEditor *pEditor, CLink *pLink, CLink NewLink)
    : IUndoCommand("Edit Link")
    , mpEditor(pEditor)
    , mpEditLink(pLink)
    , mOldLink(*pLink)
    , mNewLink(NewLink)
{
    mOldSenderIndex = pLink->SenderIndex();
    mOldReceiverIndex = pLink->ReceiverIndex();
    mAffectedInstances << pLink->Sender();
    if (pLink->Receiver() != pLink->Sender()) mAffectedInstances << pLink->Receiver();
    if (NewLink.Sender() != pLink->Sender()) mAffectedInstances << NewLink.Sender();
    if (NewLink.Receiver() != pLink->Receiver()) mAffectedInstances << NewLink.Receiver();
}

void CEditLinkCommand::undo()
{
    if (mOldLink.Sender() != mNewLink.Sender())
    {
        mNewLink.Sender()->RemoveLink(eOutgoing, mpEditLink);
        mOldLink.Sender()->AddLink(eOutgoing, mpEditLink, mOldSenderIndex);
    }
    if (mOldLink.Receiver() != mNewLink.Receiver())
    {
        mNewLink.Receiver()->RemoveLink(eIncoming, mpEditLink);
        mOldLink.Receiver()->AddLink(eIncoming, mpEditLink, mOldReceiverIndex);
    }

    *mpEditLink = mOldLink;
    mpEditor->OnLinksModified(mAffectedInstances);
}

void CEditLinkCommand::redo()
{
    if (mOldLink.Sender() != mNewLink.Sender())
    {
        mOldLink.Sender()->RemoveLink(eOutgoing, mpEditLink);
        mNewLink.Sender()->AddLink(eOutgoing, mpEditLink);
    }
    if (mOldLink.Receiver() != mNewLink.Receiver())
    {
        mOldLink.Receiver()->RemoveLink(eIncoming, mpEditLink);
        mNewLink.Receiver()->AddLink(eIncoming, mpEditLink);
    }

    *mpEditLink = mNewLink;
    mpEditor->OnLinksModified(mAffectedInstances);
}
