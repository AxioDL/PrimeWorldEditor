#include "CCloneSelectionCommand.h"
#include "Editor/CSelectionIterator.h"

CCloneSelectionCommand::CCloneSelectionCommand(INodeEditor *pEditor)
    : IUndoCommand("Clone")
    , mpEditor(qobject_cast<CWorldEditor*>(pEditor)) // todo: fix this! bad assumption! (clone handling code is in INodeEditor but active area is in CWorldEditor)
{
    mOriginalSelection = mpEditor->Selection()->SelectedNodeList();

    for (CSelectionIterator It(mpEditor->Selection()); It; ++It)
    {
        if (It->NodeType() == ENodeType::Script)
        {
            mNodesToClone.push_back(*It);

            // Fetch linked objects
            CScriptNode *pScript = static_cast<CScriptNode*>(*It);
            CScriptObject *pInst = pScript->Instance();

            for (size_t iLink = 0; iLink < pInst->NumLinks(ELinkType::Outgoing); iLink++)
            {
                CScriptNode *pNode = mpEditor->Scene()->NodeForInstance(pInst->Link(ELinkType::Outgoing, iLink)->Receiver());

                if (!pNode->IsSelected())
                    mLinkedInstances.push_back(pNode->Instance());
            }
        }
    }
}

void CCloneSelectionCommand::undo()
{
    QList<CSceneNode*> ClonedNodes = mClonedNodes.DereferenceList();
    mpEditor->Selection()->Clear();

    for (CSceneNode *pNode : ClonedNodes)
    {
        CScriptObject *pInst = static_cast<CScriptNode*>(pNode)->Instance();

        mpEditor->NotifyNodeAboutToBeDeleted(pNode);
        mpEditor->Scene()->DeleteNode(pNode);
        mpEditor->ActiveArea()->DeleteInstance(pInst);
        mpEditor->NotifyNodeDeleted();
    }

    mClonedNodes.clear();
    mpEditor->OnLinksModified(mLinkedInstances.DereferenceList());
    mpEditor->Selection()->SetSelectedNodes(mOriginalSelection.DereferenceList());
}

void CCloneSelectionCommand::redo()
{
    QList<CSceneNode*> ToClone = mNodesToClone.DereferenceList();
    QList<CSceneNode*> ClonedNodes;
    QList<uint32> ToCloneInstanceIDs;
    QList<uint32> ClonedInstanceIDs;

    // Clone nodes
    for (CSceneNode *pNode : ToClone)
    {
        mpEditor->NotifyNodeAboutToBeSpawned();
        CScriptNode *pScript = static_cast<CScriptNode*>(pNode);
        CScriptObject *pInstance = pScript->Instance();

        CScriptObject *pCloneInst = mpEditor->ActiveArea()->SpawnInstance(pInstance->Template(), pInstance->Layer());
        pCloneInst->CopyProperties(pInstance);
        pCloneInst->EvaluateProperties();

        CScriptNode *pCloneNode = mpEditor->Scene()->CreateScriptNode(pCloneInst);
        pCloneNode->SetName(pScript->Name());
        pCloneNode->SetPosition(pScript->LocalPosition());
        pCloneNode->SetRotation(pScript->LocalRotation());
        pCloneNode->SetScale(pScript->LocalScale());

        ToCloneInstanceIDs.push_back(pInstance->InstanceID());
        ClonedInstanceIDs.push_back(pCloneInst->InstanceID());
        ClonedNodes.push_back(pCloneNode);
        mClonedNodes.push_back(pCloneNode);
        mpEditor->NotifyNodeSpawned(pCloneNode);
    }

    // Clone outgoing links from source object; incoming ones are discarded
    for (int iNode = 0; iNode < ClonedNodes.size(); iNode++)
    {
        CScriptObject *pSrc = static_cast<CScriptNode*>(ToClone[iNode])->Instance();
        CScriptObject *pClone = static_cast<CScriptNode*>(ClonedNodes[iNode])->Instance();

        for (size_t iLink = 0; iLink < pSrc->NumLinks(ELinkType::Outgoing); iLink++)
        {
            CLink *pSrcLink = pSrc->Link(ELinkType::Outgoing, iLink);

            // If we're cloning the receiver then target the cloned receiver instead of the original one.
            uint32 ReceiverID = pSrcLink->ReceiverID();
            if (ToCloneInstanceIDs.contains(ReceiverID))
                ReceiverID = ClonedInstanceIDs[ToCloneInstanceIDs.indexOf(ReceiverID)];

            CLink *pCloneLink = new CLink(pSrcLink->Area(), pSrcLink->State(), pSrcLink->Message(), pClone->InstanceID(), ReceiverID);
            pCloneLink->Sender()->AddLink(ELinkType::Outgoing, pCloneLink);
            pCloneLink->Receiver()->AddLink(ELinkType::Incoming, pCloneLink);
        }
    }

    // Call LoadFinished
    for (CSceneNode *pNode : ClonedNodes)
        pNode->OnLoadFinished();

    mpEditor->OnLinksModified(mLinkedInstances.DereferenceList());
    mpEditor->Selection()->SetSelectedNodes(mClonedNodes.DereferenceList());
}
