#include "CDeleteLinksCommand.h"
#include <Core/Resource/Script/CLink.h>

CDeleteLinksCommand::CDeleteLinksCommand(CWorldEditor *pEditor, CScriptObject *pObject, ELinkType Type, const QVector<uint32>& rkIndices)
    : IUndoCommand("Delete Links")
    , mpEditor(pEditor)
{
    mAffectedInstances.push_back(pObject);

    for (const auto index : rkIndices)
    {
        const CLink *pLink = pObject->Link(Type, index);

        SDeletedLink DelLink;
        DelLink.State = pLink->State();
        DelLink.Message = pLink->Message();
        DelLink.pSender = pLink->Sender();
        DelLink.pReceiver = pLink->Receiver();
        DelLink.SenderIndex = pLink->SenderIndex();
        DelLink.ReceiverIndex = pLink->ReceiverIndex();
        mLinks.push_back(DelLink);

        if (Type == ELinkType::Outgoing)
        {
            if (!mAffectedInstances.contains(DelLink.pReceiver))
                mAffectedInstances.push_back(DelLink.pReceiver);
        }
        else
        {
            if (!mAffectedInstances.contains(DelLink.pSender))
                mAffectedInstances.push_back(DelLink.pSender);
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

    for (SDeletedLink& rDelLink : mLinks)
    {
        SNewLink Link;
        Link.pDelLink = &rDelLink;
        Link.pLink = new CLink(mpEditor->ActiveArea(), rDelLink.State, rDelLink.Message, rDelLink.pSender.InstanceID(), rDelLink.pReceiver.InstanceID());
        NewLinks.push_back(Link);
    }

    // Add to senders
    std::sort(NewLinks.begin(), NewLinks.end(), [](const SNewLink& rLinkA, const SNewLink& rLinkB) {
        return rLinkA.pDelLink->SenderIndex < rLinkB.pDelLink->SenderIndex;
    });

    for (SNewLink& rNew : NewLinks)
    {
        rNew.pDelLink->pSender->AddLink(ELinkType::Outgoing, rNew.pLink, rNew.pDelLink->SenderIndex);
    }

    // Add to receivers
    std::sort(NewLinks.begin(), NewLinks.end(), [](const SNewLink& rLinkA, const SNewLink& rLinkB) {
        return rLinkA.pDelLink->ReceiverIndex < rLinkB.pDelLink->ReceiverIndex;
    });

    for (SNewLink& rNew : NewLinks)
    {
        rNew.pDelLink->pReceiver->AddLink(ELinkType::Incoming, rNew.pLink, rNew.pDelLink->ReceiverIndex);
    }

    // Notify world editor
    mpEditor->OnLinksModified(mAffectedInstances.DereferenceList());
}

void CDeleteLinksCommand::redo()
{
    QVector<CLink*> Links;

    for (const auto& rLink : mLinks)
    {
        Links.push_back(rLink.pSender->Link(ELinkType::Outgoing, rLink.SenderIndex));
    }

    for (auto* pLink : Links)
    {
        pLink->Sender()->RemoveLink(ELinkType::Outgoing, pLink);
        pLink->Receiver()->RemoveLink(ELinkType::Incoming, pLink);
        delete pLink;
    }

    // Notify world editor
    mpEditor->OnLinksModified(mAffectedInstances.DereferenceList());
}
