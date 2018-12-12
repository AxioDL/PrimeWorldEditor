#include "CCloneSelectionCommand.h"
#include "Editor/CSelectionIterator.h"

CCloneSelectionCommand::CCloneSelectionCommand(INodeEditor *pEditor)
    : IUndoCommand("Clone")
    , mpEditor(qobject_cast<CWorldEditor*>(pEditor)) // todo: fix this! bad assumption! (clone handling code is in INodeEditor but active area is in CWorldEditor)
{
    mOriginalSelection = mpEditor->Selection()->SelectedNodeList();

    for (CSelectionIterator It(mpEditor->Selection()); It; ++It)
    {
        if (It->NodeType() == eScriptNode)
        {
            mNodesToClone << *It;

            // Fetch linked objects
            CScriptNode *pScript = static_cast<CScriptNode*>(*It);
            CScriptObject *pInst = pScript->Instance();

            for (uint32 iLink = 0; iLink < pInst->NumLinks(eOutgoing); iLink++)
            {
                CScriptNode *pNode = mpEditor->Scene()->NodeForInstance(pInst->Link(eOutgoing, iLink)->Receiver());

                if (!pNode->IsSelected())
                    mLinkedInstances << pNode->Instance();
            }
        }
    }
}

void CCloneSelectionCommand::undo()
{
    QList<CSceneNode*> ClonedNodes = mClonedNodes.DereferenceList();
    mpEditor->Selection()->Clear();

    foreach (CSceneNode *pNode, ClonedNodes)
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
    foreach (CSceneNode *pNode, ToClone)
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

        ToCloneInstanceIDs << pInstance->InstanceID();
        ClonedInstanceIDs << pCloneInst->InstanceID();
        ClonedNodes << pCloneNode;
        mClonedNodes << pCloneNode;
        mpEditor->NotifyNodeSpawned(pCloneNode);
    }

    // Clone outgoing links from source object; incoming ones are discarded
    for (int iNode = 0; iNode < ClonedNodes.size(); iNode++)
    {
        CScriptObject *pSrc = static_cast<CScriptNode*>(ToClone[iNode])->Instance();
        CScriptObject *pClone = static_cast<CScriptNode*>(ClonedNodes[iNode])->Instance();

        for (uint32 iLink = 0; iLink < pSrc->NumLinks(eOutgoing); iLink++)
        {
            CLink *pSrcLink = pSrc->Link(eOutgoing, iLink);

            // If we're cloning the receiver then target the cloned receiver instead of the original one.
            uint32 ReceiverID = pSrcLink->ReceiverID();
            if (ToCloneInstanceIDs.contains(ReceiverID))
                ReceiverID = ClonedInstanceIDs[ToCloneInstanceIDs.indexOf(ReceiverID)];

            CLink *pCloneLink = new CLink(pSrcLink->Area(), pSrcLink->State(), pSrcLink->Message(), pClone->InstanceID(), ReceiverID);
            pCloneLink->Sender()->AddLink(eOutgoing, pCloneLink);
            pCloneLink->Receiver()->AddLink(eIncoming, pCloneLink);
        }
    }

    // Call LoadFinished
    foreach (CSceneNode *pNode, ClonedNodes)
        pNode->OnLoadFinished();

    mpEditor->OnLinksModified(mLinkedInstances.DereferenceList());
    mpEditor->Selection()->SetSelectedNodes(mClonedNodes.DereferenceList());
}
