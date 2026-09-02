#include "Editor/Undo/CDeleteLinksCommand.h"

#include "Editor/WorldEditor/CWorldEditor.h"
#include <Core/Resource/Script/CLink.h>

#include <QCoreApplication>

struct CDeleteLinksCommand::SDeletedLink
{
    uint32_t State;
    uint32_t Message;
    CInstancePtr pSender;
    CInstancePtr pReceiver;
    uint32_t SenderIndex;
    uint32_t ReceiverIndex;
};

CDeleteLinksCommand::CDeleteLinksCommand() = default;

CDeleteLinksCommand::CDeleteLinksCommand(CWorldEditor *pEditor, CScriptObject *pObject, ELinkType Type, std::span<uint32_t> indices)
    : IUndoCommand(QCoreApplication::translate("CDeleteLinksCommand", "Delete Links"))
    , mpEditor(pEditor)
{
    mAffectedInstances.push_back(pObject);

    for (const auto index : indices)
    {
        const CLink* pLink = pObject->Link(Type, index);
        mLinks.push_back({
            .State = pLink->State(),
            .Message = pLink->Message(),
            .pSender = pLink->Sender(),
            .pReceiver = pLink->Receiver(),
            .SenderIndex = pLink->SenderIndex(),
            .ReceiverIndex = pLink->ReceiverIndex(),
        });

        const auto& DelLink = mLinks.back();
        const auto& Instance = Type == ELinkType::Outgoing ? DelLink.pReceiver : DelLink.pSender;
        if (!mAffectedInstances.contains(Instance))
            mAffectedInstances.push_back(Instance);
    }
}

CDeleteLinksCommand::~CDeleteLinksCommand() = default;

void CDeleteLinksCommand::undo()
{
    struct SNewLink
    {
        SDeletedLink *pDelLink;
        CLink *pLink;

        static bool SenderIndexSorter(const SNewLink& l, const SNewLink& r) {
            return l.pDelLink->SenderIndex < r.pDelLink->SenderIndex;
        }
        static bool ReceiverIndexSorter(const SNewLink& l, const SNewLink& r) {
            return l.pDelLink->ReceiverIndex < r.pDelLink->ReceiverIndex;
        }
    };

    QList<SNewLink> NewLinks;
    for (SDeletedLink& rDelLink : mLinks)
    {
        NewLinks.push_back({
            .pDelLink = &rDelLink,
            .pLink = new CLink(mpEditor->ActiveArea(), rDelLink.State, rDelLink.Message, rDelLink.pSender.InstanceID(), rDelLink.pReceiver.InstanceID()),
        });
    }

    // Add to senders
    std::ranges::sort(NewLinks, &SNewLink::SenderIndexSorter);
    for (SNewLink& rNew : NewLinks)
    {
        rNew.pDelLink->pSender->AddLink(ELinkType::Outgoing, rNew.pLink, rNew.pDelLink->SenderIndex);
    }

    // Add to receivers
    std::ranges::sort(NewLinks, &SNewLink::ReceiverIndexSorter);
    for (SNewLink& rNew : NewLinks)
    {
        rNew.pDelLink->pReceiver->AddLink(ELinkType::Incoming, rNew.pLink, rNew.pDelLink->ReceiverIndex);
    }

    // Notify world editor
    mpEditor->OnLinksModified(mAffectedInstances.DereferenceList());
}

void CDeleteLinksCommand::redo()
{
    QList<CLink*> Links;

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
