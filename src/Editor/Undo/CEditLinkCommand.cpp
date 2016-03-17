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
    if (pLink->ReceiverID() != pLink->SenderID())       mAffectedInstances << pLink->Receiver();
    if (NewLink.SenderID() != pLink->SenderID())        mAffectedInstances << NewLink.Sender();
    if (NewLink.ReceiverID() != pLink->ReceiverID())    mAffectedInstances << NewLink.Receiver();
}

void CEditLinkCommand::undo()
{
    CLink *pLink = *mpEditLink;

    if (mOldLink.Sender() != mNewLink.Sender())
    {
        mNewLink.Sender()->RemoveLink(eOutgoing, pLink);
        mOldLink.Sender()->AddLink(eOutgoing, pLink, mOldSenderIndex);
    }
    if (mOldLink.Receiver() != mNewLink.Receiver())
    {
        mNewLink.Receiver()->RemoveLink(eIncoming, pLink);
        mOldLink.Receiver()->AddLink(eIncoming, pLink, mOldReceiverIndex);
    }

    *pLink = mOldLink;
    mpEditor->OnLinksModified(mAffectedInstances.DereferenceList());
    mpEditLink.SetLink(pLink); // note: This is done to account for situations where the sender has changed
}

void CEditLinkCommand::redo()
{
    CLink *pLink = *mpEditLink;

    if (mOldLink.Sender() != mNewLink.Sender())
    {
        mOldLink.Sender()->RemoveLink(eOutgoing, pLink);
        mNewLink.Sender()->AddLink(eOutgoing, pLink);
    }
    if (mOldLink.Receiver() != mNewLink.Receiver())
    {
        mOldLink.Receiver()->RemoveLink(eIncoming, pLink);
        mNewLink.Receiver()->AddLink(eIncoming, pLink);
    }

    *pLink = mNewLink;
    mpEditor->OnLinksModified(mAffectedInstances.DereferenceList());
    mpEditLink.SetLink(pLink);
}
