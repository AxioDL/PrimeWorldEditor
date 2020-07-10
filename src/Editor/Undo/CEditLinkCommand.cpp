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

    mAffectedInstances.push_back(pLink->Sender());

    if (pLink->ReceiverID() != pLink->SenderID())
        mAffectedInstances.push_back(pLink->Receiver());

    if (NewLink.SenderID() != pLink->SenderID())
        mAffectedInstances.push_back(NewLink.Sender());

    if (NewLink.ReceiverID() != pLink->ReceiverID())
        mAffectedInstances.push_back(NewLink.Receiver());
}

void CEditLinkCommand::undo()
{
    CLink *pLink = *mpEditLink;

    if (mOldLink.Sender() != mNewLink.Sender())
    {
        mNewLink.Sender()->RemoveLink(ELinkType::Outgoing, pLink);
        mOldLink.Sender()->AddLink(ELinkType::Outgoing, pLink, mOldSenderIndex);
    }
    if (mOldLink.Receiver() != mNewLink.Receiver())
    {
        mNewLink.Receiver()->RemoveLink(ELinkType::Incoming, pLink);
        mOldLink.Receiver()->AddLink(ELinkType::Incoming, pLink, mOldReceiverIndex);
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
        mOldLink.Sender()->RemoveLink(ELinkType::Outgoing, pLink);
        mNewLink.Sender()->AddLink(ELinkType::Outgoing, pLink);
    }
    if (mOldLink.Receiver() != mNewLink.Receiver())
    {
        mOldLink.Receiver()->RemoveLink(ELinkType::Incoming, pLink);
        mNewLink.Receiver()->AddLink(ELinkType::Incoming, pLink);
    }

    *pLink = mNewLink;
    mpEditor->OnLinksModified(mAffectedInstances.DereferenceList());
    mpEditLink.SetLink(pLink);
}
