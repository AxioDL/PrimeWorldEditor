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
            mNodesToClone << *It;
    }
}

void CCloneSelectionCommand::undo()
{
    QList<CSceneNode*> ClonedNodes = mClonedNodes.DereferenceList();
    mpEditor->Selection()->Clear();

    foreach (CSceneNode *pNode, ClonedNodes)
    {
        CScriptObject *pInst = static_cast<CScriptNode*>(pNode)->Object();

        mpEditor->NotifyNodeAboutToBeDeleted(pNode);
        mpEditor->Scene()->DeleteNode(pNode);
        mpEditor->ActiveArea()->DeleteInstance(pInst);
        mpEditor->NotifyNodeDeleted();
    }

    mClonedNodes.clear();
    mpEditor->Selection()->SetSelectedNodes(mOriginalSelection.DereferenceList());
}

void CCloneSelectionCommand::redo()
{
    QList<CSceneNode*> ToClone = mNodesToClone.DereferenceList();
    QList<CSceneNode*> ClonedNodes;
    QList<u32> ToCloneInstanceIDs;
    QList<u32> ClonedInstanceIDs;

    foreach (CSceneNode *pNode, ToClone)
    {
        mpEditor->NotifyNodeAboutToBeSpawned();
        CScriptNode *pScript = static_cast<CScriptNode*>(pNode);
        CScriptObject *pInstance = pScript->Object();

        CScriptObject *pCloneInst = mpEditor->ActiveArea()->SpawnInstance(pInstance->Template(), pInstance->Layer());
        pCloneInst->Properties()->Copy(pInstance->Properties());
        pCloneInst->EvaluateProperties();

        CScriptNode *pCloneNode = mpEditor->Scene()->CreateScriptNode(pCloneInst);
        pCloneNode->SetName(pScript->Name());
        pCloneNode->SetPosition(pScript->LocalPosition());
        pCloneNode->SetRotation(pScript->LocalRotation());
        pCloneNode->SetScale(pScript->LocalScale());
        pCloneNode->OnLoadFinished();

        ToCloneInstanceIDs << pInstance->InstanceID();
        ClonedInstanceIDs << pCloneInst->InstanceID();
        ClonedNodes << pCloneNode;
        mClonedNodes << pCloneNode;
        mpEditor->NotifyNodeSpawned(pCloneNode);
    }

    // Clone outgoing links from source object; incoming ones are discarded
    QList<CScriptObject*> LinkedInstances;

    for (int iNode = 0; iNode < ClonedNodes.size(); iNode++)
    {
        CScriptObject *pSrc = static_cast<CScriptNode*>(ToClone[iNode])->Object();
        CScriptObject *pClone = static_cast<CScriptNode*>(ClonedNodes[iNode])->Object();

        for (u32 iLink = 0; iLink < pSrc->NumLinks(eOutgoing); iLink++)
        {
            CLink *pSrcLink = pSrc->Link(eOutgoing, iLink);

            // If we're cloning the receiver then target the cloned receiver instead of the original one.
            u32 ReceiverID = pSrcLink->ReceiverID();
            if (ToCloneInstanceIDs.contains(ReceiverID))
                ReceiverID = ClonedInstanceIDs[ToCloneInstanceIDs.indexOf(ReceiverID)];

            CLink *pCloneLink = new CLink(pSrcLink->Area(), pSrcLink->State(), pSrcLink->Message(), pClone->InstanceID(), ReceiverID);
            pCloneLink->Sender()->AddLink(eOutgoing, pCloneLink);
            pCloneLink->Receiver()->AddLink(eIncoming, pCloneLink);

            if (!LinkedInstances.contains(pCloneLink->Sender()))
                LinkedInstances << pCloneLink->Sender();
            if (!LinkedInstances.contains(pCloneLink->Receiver()))
                LinkedInstances << pCloneLink->Receiver();
        }
    }

    mpEditor->OnLinksModified(LinkedInstances);
    mpEditor->Selection()->SetSelectedNodes(mClonedNodes.DereferenceList());
}
