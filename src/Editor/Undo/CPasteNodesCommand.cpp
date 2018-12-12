#include "CPasteNodesCommand.h"

CPasteNodesCommand::CPasteNodesCommand(CWorldEditor *pEditor, CScriptLayer *pLayer, CVector3f PastePoint)
    : IUndoCommand("Paste")
    , mpEditor(pEditor)
    , mpLayer(pLayer)
    , mPastePoint(PastePoint)
    , mOriginalSelection(pEditor->Selection()->SelectedNodeList())
{
    const CNodeCopyMimeData *pkMimeData = qobject_cast<const CNodeCopyMimeData*>(qApp->clipboard()->mimeData());

    if (pkMimeData)
        mpMimeData = new CNodeCopyMimeData(*pkMimeData);
    else
        mpMimeData = nullptr;
}

CPasteNodesCommand::~CPasteNodesCommand()
{
    if (mpMimeData) delete mpMimeData;
}

void CPasteNodesCommand::undo()
{
    mpEditor->Selection()->SetSelectedNodes(mOriginalSelection.DereferenceList());
    QList<CSceneNode*> PastedNodes = mPastedNodes.DereferenceList();

    foreach (CSceneNode *pNode, PastedNodes)
    {
        CScriptObject *pInst = (pNode->NodeType() == eScriptNode ? static_cast<CScriptNode*>(pNode)->Instance() : nullptr);
        mpEditor->NotifyNodeAboutToBeDeleted(pNode);
        mpEditor->Scene()->DeleteNode(pNode);
        if (pInst) mpEditor->ActiveArea()->DeleteInstance(pInst);
        mpEditor->NotifyNodeDeleted();
    }

    mpEditor->OnLinksModified(mLinkedInstances.DereferenceList());
    mLinkedInstances.clear();
    mPastedNodes.clear();
}

void CPasteNodesCommand::redo()
{
    if (!mpMimeData) return;

    const QVector<CNodeCopyMimeData::SCopiedNode>& rkNodes = mpMimeData->CopiedNodes();
    CScene *pScene = mpEditor->Scene();
    CGameArea *pArea = mpEditor->ActiveArea();
    QList<CSceneNode*> PastedNodes;

    foreach (const CNodeCopyMimeData::SCopiedNode& rkNode, rkNodes)
    {
        CSceneNode *pNewNode = nullptr;

        if (rkNode.Type == eScriptNode)
        {
            CMemoryInStream In(rkNode.InstanceData.data(), rkNode.InstanceData.size(), IOUtil::eBigEndian);
            CScriptObject *pInstance = CScriptLoader::LoadInstance(In, pArea, mpLayer, pArea->Game(), false);
            pArea->AddInstanceToArea(pInstance);
            mpLayer->AddInstance(pInstance);

            pInstance->SetPosition(rkNode.Position + mPastePoint);
            pInstance->SetRotation(rkNode.Rotation.ToEuler());
            pInstance->SetScale(rkNode.Scale);

            mpEditor->NotifyNodeAboutToBeSpawned();
            pNewNode = pScene->CreateScriptNode(pInstance);
        }

        if (pNewNode)
        {
            pNewNode->SetName(rkNode.Name);
            pNewNode->SetPosition(rkNode.Position + mPastePoint);
            pNewNode->SetRotation(rkNode.Rotation);
            pNewNode->SetScale(rkNode.Scale);

            PastedNodes << pNewNode;
            mpEditor->NotifyNodeSpawned(pNewNode);
        }

        // If we didn't paste a valid node, add a null node so that the indices still match up with the indices from the mime data.
        else
            PastedNodes << nullptr;
    }

    // Fix links. This is how fixes are prioritized:
    // 1. If the link receiver has also been copied then redirect to the copied version.
    // 2. If we're pasting into the same area that this data was copied from and the receiver still exists, connect to original receiver.
    // 3. If neither of those things is true, then delete the link.
    foreach (CSceneNode *pNode, PastedNodes)
    {
        if (pNode && pNode->NodeType() == eScriptNode)
        {
            CScriptObject *pInstance = static_cast<CScriptNode*>(pNode)->Instance();

            for (uint32 iLink = 0; iLink < pInstance->NumLinks(eOutgoing); iLink++)
            {
                CLink *pLink = pInstance->Link(eOutgoing, iLink);
                int Index = mpMimeData->IndexOfInstanceID(pLink->ReceiverID());

                if (Index != -1)
                {
                    CScriptObject *pNewTarget = static_cast<CScriptNode*>(PastedNodes[Index])->Instance();
                    pLink->SetReceiver(pNewTarget->InstanceID());
                }

                else if (mpMimeData->AreaID() != pArea->ID() || pArea->InstanceByID(pLink->ReceiverID()) == nullptr)
                {
                    CScriptObject *pSender = pLink->Sender();
                    CScriptObject *pReceiver = pLink->Receiver();
                    if (pSender) pSender->RemoveLink(eOutgoing, pLink);
                    if (pReceiver) pReceiver->RemoveLink(eIncoming, pLink);

                    delete pLink;
                    iLink--;
                }

                else
                {
                    CScriptObject *pReceiver = pLink->Receiver();
                    pReceiver->AddLink(eIncoming, pLink);
                    mLinkedInstances << pReceiver;
                }
            }
        }
    }

    // Call PostLoad on all new nodes and select them
    PastedNodes.removeAll(nullptr);

    foreach (CSceneNode *pNode, PastedNodes)
        pNode->OnLoadFinished();

    mpEditor->Selection()->SetSelectedNodes(PastedNodes);

    mpEditor->OnLinksModified(mLinkedInstances.DereferenceList());
    mPastedNodes = PastedNodes;
}
