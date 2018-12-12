#include "CDeleteSelectionCommand.h"
#include "Editor/CSelectionIterator.h"
#include <Common/FileIO.h>
#include <Core/Resource/Cooker/CScriptCooker.h>
#include <Core/Resource/Factory/CScriptLoader.h>

CDeleteSelectionCommand::CDeleteSelectionCommand(CWorldEditor *pEditor, const QString& rkCommandName /*= "Delete"*/)
    : IUndoCommand(rkCommandName)
    , mpEditor(pEditor)
{
    QSet<CLink*> Links;
    QList<CScriptObject*> LinkedInstances;

    for (CSelectionIterator It(pEditor->Selection()); It; ++It)
    {
        mOldSelection << *It;

        if (It->NodeType() == eScriptNode)
        {
            CScriptNode *pScript = static_cast<CScriptNode*>(*It);
            CScriptObject *pInst = pScript->Instance();

            mDeletedNodes.push_back(SDeletedNode());
            SDeletedNode& rNode = mDeletedNodes.back();

            rNode.NodePtr = CNodePtr(pScript);
            rNode.NodeID = pScript->ID();
            rNode.Position = pScript->LocalPosition();
            rNode.Rotation = pScript->LocalRotation();
            rNode.Scale = pScript->LocalScale();

            rNode.pArea = pInst->Area();
            rNode.pLayer = pInst->Layer();
            rNode.LayerIndex = pInst->LayerIndex();

            for (uint32 iType = 0; iType < 2; iType++)
            {
                ELinkType Type = (iType == 0 ? eOutgoing : eIncoming);

                for (uint32 iLink = 0; iLink < pInst->NumLinks(Type); iLink++)
                {
                    CLink *pLink = pInst->Link(Type, iLink);

                    if (!Links.contains(pLink))
                    {
                        SDeletedLink Link;
                        Link.State = pLink->State();
                        Link.Message = pLink->Message();
                        Link.SenderID = pLink->SenderID();
                        Link.SenderIndex = pLink->SenderIndex();
                        Link.ReceiverID = pLink->ReceiverID();
                        Link.ReceiverIndex = pLink->ReceiverIndex();
                        Link.pSender = pLink->Sender();
                        Link.pReceiver = pLink->Receiver();
                        mDeletedLinks << Link;
                        Links << pLink;

                        if (!LinkedInstances.contains(pLink->Sender()))
                            LinkedInstances << pLink->Sender();
                        if (!LinkedInstances.contains(pLink->Receiver()))
                            LinkedInstances << pLink->Receiver();
                    }
                }
            }

            CVectorOutStream PropertyDataOut(&rNode.InstanceData, IOUtil::eBigEndian);
            CScriptCooker Cooker(pEditor->CurrentGame());
            Cooker.WriteInstance(PropertyDataOut, pInst);
        }

        else
            mNewSelection << *It;
    }

    // Remove selected objects from the linked instances list.
    LinkedInstances.removeAll(nullptr);

    foreach (CScriptObject *pInst, LinkedInstances)
    {
        if (mpEditor->Scene()->NodeForInstance(pInst)->IsSelected())
            LinkedInstances.removeOne(pInst);
    }

    mLinkedInstances = LinkedInstances;
}

void CDeleteSelectionCommand::undo()
{
    QList<CSceneNode*> NewNodes;
    QList<uint32> NewInstanceIDs;

    // Spawn nodes
    for (int iNode = 0; iNode < mDeletedNodes.size(); iNode++)
    {
        SDeletedNode& rNode = mDeletedNodes[iNode];
        mpEditor->NotifyNodeAboutToBeSpawned();

        CMemoryInStream Mem(rNode.InstanceData.data(), rNode.InstanceData.size(), IOUtil::eBigEndian);
        CScriptObject *pInstance = CScriptLoader::LoadInstance(Mem, rNode.pArea, rNode.pLayer, rNode.pArea->Game(), true);
        CScriptNode *pNode = mpEditor->Scene()->CreateScriptNode(pInstance, rNode.NodeID);
        rNode.pArea->AddInstanceToArea(pInstance);
        rNode.pLayer->AddInstance(pInstance, rNode.LayerIndex);

        pNode->SetPosition(rNode.Position);
        pNode->SetRotation(rNode.Rotation);
        pNode->SetScale(rNode.Scale);

        NewNodes << pNode;
        NewInstanceIDs << pInstance->InstanceID();
        mpEditor->NotifyNodeSpawned(*rNode.NodePtr);
    }

    // Sort links by sender index, add outgoing
    qSort(mDeletedLinks.begin(), mDeletedLinks.end(), [](SDeletedLink& rLeft, SDeletedLink& rRight) -> bool {
        return (rLeft.SenderIndex < rRight.SenderIndex);
    });

    for (int iLink = 0; iLink < mDeletedLinks.size(); iLink++)
    {
        SDeletedLink& rLink = mDeletedLinks[iLink];

        // Adding to the sender is only needed if the sender is not one of the nodes we just spawned. If it is, it already has this link.
        if (!NewInstanceIDs.contains(rLink.SenderID) && *rLink.pSender)
        {
            CLink *pLink = new CLink(rLink.pSender->Area(), rLink.State, rLink.Message, rLink.SenderID, rLink.ReceiverID);
            rLink.pSender->AddLink(eOutgoing, pLink, rLink.SenderIndex);
        }
    }

    // Sort links by receiver index, add incoming
    qSort(mDeletedLinks.begin(), mDeletedLinks.end(), [](SDeletedLink& rLeft, SDeletedLink& rRight) -> bool {
        return (rLeft.ReceiverIndex < rRight.ReceiverIndex);
    });

    for (int iLink = 0; iLink < mDeletedLinks.size(); iLink++)
    {
        SDeletedLink& rLink = mDeletedLinks[iLink];

        if (*rLink.pReceiver)
        {
            CLink *pLink = (*rLink.pSender ? rLink.pSender->Link(eOutgoing, rLink.SenderIndex) : new CLink(rLink.pReceiver->Area(), rLink.State, rLink.Message, rLink.SenderID, rLink.ReceiverID));
            rLink.pReceiver->AddLink(eIncoming, pLink, rLink.ReceiverIndex);
        }
    }

    // Run OnLoadFinished
    foreach (CSceneNode *pNode, NewNodes)
        pNode->OnLoadFinished();

    // Add selection and done
    mpEditor->Selection()->SetSelectedNodes(mOldSelection.DereferenceList());
    mpEditor->OnLinksModified(mLinkedInstances.DereferenceList());
}

void CDeleteSelectionCommand::redo()
{
    mpEditor->Selection()->SetSelectedNodes(mNewSelection.DereferenceList());

    for (int iNode = 0; iNode < mDeletedNodes.size(); iNode++)
    {
        SDeletedNode& rNode = mDeletedNodes[iNode];
        CSceneNode *pNode = *rNode.NodePtr;
        CScriptObject *pInst = static_cast<CScriptNode*>(pNode)->Instance();

        mpEditor->NotifyNodeAboutToBeDeleted(pNode);
        mpEditor->Scene()->DeleteNode(pNode);
        mpEditor->ActiveArea()->DeleteInstance(pInst);
        mpEditor->NotifyNodeDeleted();
    }

    mpEditor->OnLinksModified(mLinkedInstances.DereferenceList());
}
