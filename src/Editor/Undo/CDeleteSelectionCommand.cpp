#include "Editor/Undo/CDeleteSelectionCommand.h"

#include "Editor/WorldEditor/CWorldEditor.h"
#include <Core/Resource/Cooker/CScriptCooker.h>
#include <Core/Resource/Factory/CScriptLoader.h>
#include <Core/Resource/Script/CScriptLayer.h>
#include <Core/Scene/CSceneNode.h>

#include <Common/FileIO/CMemoryInStream.h>
#include <Common/FileIO/CVectorOutStream.h>

#include <vector>

struct CDeleteSelectionCommand::SDeletedNode
{
    // Node Info
    CNodePtr NodePtr;
    uint32_t NodeID;
    CVector3f Position;
    CQuaternion Rotation;
    CVector3f Scale;

    // Instance Info
    CGameArea* pArea;
    CScriptLayer* pLayer;
    uint32_t LayerIndex;
    std::vector<char> InstanceData;
};

struct CDeleteSelectionCommand::SDeletedLink
{
    uint32_t State;
    uint32_t Message;
    CInstanceID SenderID;
    uint32_t SenderIndex;
    CInstanceID ReceiverID;
    uint32_t ReceiverIndex;
    CInstancePtr pSender;
    CInstancePtr pReceiver;
};

CDeleteSelectionCommand::CDeleteSelectionCommand(CWorldEditor *pEditor, const QString& rkCommandName)
    : IUndoCommand(rkCommandName)
    , mpEditor(pEditor)
{
    QSet<CLink*> Links;
    QList<CScriptObject*> LinkedInstances;

    for (auto* node : pEditor->Selection()->Nodes())
    {
        mOldSelection.push_back(node);

        if (node->NodeType() == ENodeType::Script)
        {
            auto* pScript = static_cast<CScriptNode*>(node);
            auto* pInst = pScript->Instance();
            auto& rNode = mDeletedNodes.emplace_back();

            rNode.NodePtr = CNodePtr(pScript);
            rNode.NodeID = pScript->ID();
            rNode.Position = pScript->LocalPosition();
            rNode.Rotation = pScript->LocalRotation();
            rNode.Scale = pScript->LocalScale();

            rNode.pArea = pInst->Area();
            rNode.pLayer = pInst->Layer();
            rNode.LayerIndex = pInst->LayerIndex();

            for (const auto type : {ELinkType::Outgoing, ELinkType::Incoming})
            {
                for (CLink* link : pInst->Links(type))
                {
                    if (!Links.contains(link))
                    {
                        mDeletedLinks.push_back({
                            .State = link->State(),
                            .Message = link->Message(),
                            .SenderID = link->SenderID(),
                            .SenderIndex = link->SenderIndex(),
                            .ReceiverID = link->ReceiverID(),
                            .ReceiverIndex = link->ReceiverIndex(),
                            .pSender = link->Sender(),
                            .pReceiver = link->Receiver(),
                        });
                        Links.insert(link);

                        if (!LinkedInstances.contains(link->Sender()))
                            LinkedInstances.push_back(link->Sender());
                        if (!LinkedInstances.contains(link->Receiver()))
                            LinkedInstances.push_back(link->Receiver());
                    }
                }
            }

            CVectorOutStream PropertyDataOut(&rNode.InstanceData, std::endian::big);
            CScriptCooker Cooker(pEditor->CurrentGame());
            Cooker.WriteInstance(PropertyDataOut, pInst);
        }
        else
        {
            mNewSelection.push_back(node);
        }
    }

    // Remove selected objects from the linked instances list.
    LinkedInstances.removeAll(nullptr);

    for (const CScriptObject* pInst : LinkedInstances)
    {
        if (mpEditor->Scene()->NodeForInstance(pInst)->IsSelected())
            LinkedInstances.removeOne(pInst);
    }

    mLinkedInstances = LinkedInstances;
}

CDeleteSelectionCommand::~CDeleteSelectionCommand() = default;

void CDeleteSelectionCommand::undo()
{
    QList<CSceneNode*> NewNodes;
    QList<CInstanceID> NewInstanceIDs;

    // Spawn nodes
    for (SDeletedNode& rNode : mDeletedNodes)
    {
        mpEditor->NotifyNodeAboutToBeSpawned();

        CMemoryInStream Mem(rNode.InstanceData.data(), rNode.InstanceData.size(), std::endian::big);
        CScriptObject *pInstance = CScriptLoader::LoadInstance(Mem, rNode.pArea, rNode.pLayer, rNode.pArea->Game(), rNode.pArea->Game() > EGame::Prime);
        CScriptNode *pNode = mpEditor->Scene()->CreateScriptNode(pInstance, rNode.NodeID);
        rNode.pArea->AddInstanceToArea(pInstance);
        rNode.pLayer->AddInstance(pInstance, rNode.LayerIndex);

        pNode->SetPosition(rNode.Position);
        pNode->SetRotation(rNode.Rotation);
        pNode->SetScale(rNode.Scale);

        NewNodes.push_back(pNode);
        NewInstanceIDs.push_back(pInstance->InstanceID());
        mpEditor->NotifyNodeSpawned(*rNode.NodePtr);
    }

    // Sort links by sender index, add outgoing
    std::ranges::sort(mDeletedLinks, [](const SDeletedLink& rLeft, const SDeletedLink& rRight) {
        return rLeft.SenderIndex < rRight.SenderIndex;
    });

    for (SDeletedLink& rLink : mDeletedLinks)
    {
        // Adding to the sender is only needed if the sender is not one of the nodes we just spawned. If it is, it already has this link.
        if (!NewInstanceIDs.contains(rLink.SenderID) && *rLink.pSender)
        {
            CLink *pLink = new CLink(rLink.pSender->Area(), rLink.State, rLink.Message, rLink.SenderID, rLink.ReceiverID);
            rLink.pSender->AddLink(ELinkType::Outgoing, pLink, rLink.SenderIndex);
        }
    }

    // Sort links by receiver index, add incoming
    std::ranges::sort(mDeletedLinks, [](const SDeletedLink& rLeft, const SDeletedLink& rRight) {
        return rLeft.ReceiverIndex < rRight.ReceiverIndex;
    });

    for (SDeletedLink& rLink : mDeletedLinks)
    {
        if (*rLink.pReceiver)
        {
            CLink *pLink = (*rLink.pSender ? rLink.pSender->Link(ELinkType::Outgoing, rLink.SenderIndex) : new CLink(rLink.pReceiver->Area(), rLink.State, rLink.Message, rLink.SenderID, rLink.ReceiverID));
            rLink.pReceiver->AddLink(ELinkType::Incoming, pLink, rLink.ReceiverIndex);
        }
    }

    // Run OnLoadFinished
    for (CSceneNode *pNode : NewNodes)
        pNode->OnLoadFinished();

    // Add selection and done
    mpEditor->Selection()->SetSelectedNodes(mOldSelection.DereferenceList());
    mpEditor->OnLinksModified(mLinkedInstances.DereferenceList());
}

void CDeleteSelectionCommand::redo()
{
    mpEditor->Selection()->SetSelectedNodes(mNewSelection.DereferenceList());

    for (SDeletedNode& rNode : mDeletedNodes)
    {
        CSceneNode *pNode = *rNode.NodePtr;
        CScriptObject *pInst = static_cast<CScriptNode*>(pNode)->Instance();

        mpEditor->NotifyNodeAboutToBeDeleted(pNode);
        mpEditor->Scene()->DeleteNode(pNode);
        mpEditor->ActiveArea()->DeleteInstance(pInst);
        mpEditor->NotifyNodeDeleted();
    }

    mpEditor->OnLinksModified(mLinkedInstances.DereferenceList());
}
