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
}

void CCreateInstanceCommand::undo()
{
    mpEditor->NotifyNodeAboutToBeDeleted(mpNewNode);
    mpEditor->Selection()->SetSelectedNodes(mOldSelection);
    mpScene->DeleteNode(mpNewNode);
    mpArea->DeleteInstance(mpNewInstance);
    mpNewNode = nullptr;
    mpNewInstance = nullptr;
    mpEditor->NotifyNodeDeleted();
}

void CCreateInstanceCommand::redo()
{
    mpEditor->NotifyNodeAboutToBeSpawned();

    CScriptLayer *pLayer = (mLayerIndex == -1 ? mpArea->GetGeneratorLayer() : mpArea->GetScriptLayer(mLayerIndex));
    mpNewInstance = mpArea->SpawnInstance(mpTemplate, pLayer, mSpawnPosition);
    mpNewNode = mpScene->CreateScriptNode(mpNewInstance);
    mpNewNode->SetPosition(mSpawnPosition);

    mpEditor->NotifyNodeSpawned(mpNewNode);
    mpEditor->Selection()->ClearAndSelectNode(mpNewNode);
}
