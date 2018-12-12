#include "CDeleteLinksCommand.h"
#include <Core/Resource/Script/CLink.h>

CDeleteLinksCommand::CDeleteLinksCommand(CWorldEditor *pEditor, CScriptObject *pObject, ELinkType Type, const QVector<uint32>& rkIndices)
    : IUndoCommand("Delete Links")
    , mpEditor(pEditor)
{
    mAffectedInstances << pObject;

    for (int iIdx = 0; iIdx < rkIndices.size(); iIdx++)
    {
        CLink *pLink = pObject->Link(Type, rkIndices[iIdx]);

        SDeletedLink DelLink;
        DelLink.State = pLink->State();
        DelLink.Message = pLink->Message();
        DelLink.pSender = pLink->Sender();
        DelLink.pReceiver = pLink->Receiver();
        DelLink.SenderIndex = pLink->SenderIndex();
        DelLink.ReceiverIndex = pLink->ReceiverIndex();
        mLinks << DelLink;

        if (Type == eOutgoing)
        {
            if (!mAffectedInstances.contains(DelLink.pReceiver))
                mAffectedInstances << DelLink.pReceiver;
        }
        else
        {
            if (!mAffectedInstances.contains(DelLink.pSender))
                mAffectedInstances << DelLink.pSender;
        }
    }
}

void CDeleteLinksCommand::undo()
{
    struct SNewLink
    {
        SDeletedLink *pDelLink;
        CLink *pLink;
    };
    QVector<SNewLink> NewLinks;

    for (int iLink = 0; iLink < mLinks.size(); iLink++)
    {
        SDeletedLink& rDelLink = mLinks[iLink];

        SNewLink Link;
        Link.pDelLink = &mLinks[iLink];
        Link.pLink = new CLink(mpEditor->ActiveArea(), rDelLink.State, rDelLink.Message, rDelLink.pSender.InstanceID(), rDelLink.pReceiver.InstanceID());
        NewLinks << Link;
    }

    // Add to senders
    qSort(NewLinks.begin(), NewLinks.end(), [](SNewLink& rLinkA, SNewLink& rLinkB) { return rLinkA.pDelLink->SenderIndex < rLinkB.pDelLink->SenderIndex; });

    for (int iLink = 0; iLink < NewLinks.size(); iLink++)
    {
        SNewLink& rNew = NewLinks[iLink];
        rNew.pDelLink->pSender->AddLink(eOutgoing, rNew.pLink, rNew.pDelLink->SenderIndex);
    }

    // Add to receivers
    qSort(NewLinks.begin(), NewLinks.end(), [](SNewLink& rLinkA, SNewLink& rLinkB) { return rLinkA.pDelLink->ReceiverIndex < rLinkB.pDelLink->ReceiverIndex; });

    for (int iLink = 0; iLink < NewLinks.size(); iLink++)
    {
        SNewLink& rNew = NewLinks[iLink];
        rNew.pDelLink->pReceiver->AddLink(eIncoming, rNew.pLink, rNew.pDelLink->ReceiverIndex);
    }

    // Notify world editor
    mpEditor->OnLinksModified(mAffectedInstances.DereferenceList());
}

void CDeleteLinksCommand::redo()
{
    QVector<CLink*> Links;

    for (int iLink = 0; iLink < mLinks.size(); iLink++)
    {
        SDeletedLink& rLink = mLinks[iLink];
        Links << rLink.pSender->Link(eOutgoing, rLink.SenderIndex);
    }

    for (int iLink = 0; iLink < Links.size(); iLink++)
    {
        CLink *pLink = Links[iLink];
        pLink->Sender()->RemoveLink(eOutgoing, pLink);
        pLink->Receiver()->RemoveLink(eIncoming, pLink);
        delete pLink;
    }

    // Notify world editor
    mpEditor->OnLinksModified(mAffectedInstances.DereferenceList());
}
