#include "CCreateInstanceCommand.h"

CCreateInstanceCommand::CCreateInstanceCommand(CWorldEditor *pEditor, CScriptTemplate *pTemplate, CScriptLayer *pLayer, const CVector3f& rkPosition)
    : IUndoCommand("Create Instance")
    , mpEditor(pEditor)
    , mpScene(pEditor->Scene())
    , mpArea(pEditor->ActiveArea())
    , mpTemplate(pTemplate)
    , mLayerIndex(pLayer->AreaIndex())
    , mSpawnPosition(rkPosition)
    , mOldSelection(pEditor->Selection()->SelectedNodeList())
    , mpNewInstance(nullptr)
    , mpNewNode(nullptr)
{
    ASSERT(mLayerIndex != UINT32_MAX);
}

void CCreateInstanceCommand::undo()
{
    mpEditor->NotifyNodeAboutToBeDeleted(*mpNewNode);
    mpEditor->Selection()->SetSelectedNodes(mOldSelection.DereferenceList());
    mpScene->DeleteNode(*mpNewNode);
    mpArea->DeleteInstance(*mpNewInstance);
    mpEditor->NotifyNodeDeleted();

    mpNewNode = nullptr;
    mpNewInstance = nullptr;
}

void CCreateInstanceCommand::redo()
{
    mpEditor->NotifyNodeAboutToBeSpawned();

    CScriptLayer *pLayer = mpArea->ScriptLayer(mLayerIndex);
    CScriptObject *pNewInst = mpArea->SpawnInstance(mpTemplate, pLayer, mSpawnPosition);
    CScriptNode *pNewNode = mpScene->CreateScriptNode(pNewInst);
    pNewNode->SetPosition(mSpawnPosition);
    pNewNode->OnLoadFinished();

    mpEditor->NotifyNodeSpawned(pNewNode);
    mpEditor->Selection()->ClearAndSelectNode(pNewNode);

    mpNewInstance = pNewInst;
    mpNewNode = pNewNode;
}
