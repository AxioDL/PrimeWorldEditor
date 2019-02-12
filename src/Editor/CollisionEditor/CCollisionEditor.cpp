#include "CCollisionEditor.h"
#include "ui_CCollisionEditor.h"

CCollisionEditor::CCollisionEditor(CCollisionMeshGroup* pCollisionMesh, QWidget* pParent /*= 0*/)
    : IEditor(pParent)
    , mpUI(new Ui::CCollisionEditor)
{
    mpUI = std::make_unique<Ui::CCollisionEditor>();
    mpUI->setupUi(this);

    mpCollisionMesh = pCollisionMesh;

    mpScene = std::make_unique<CScene>();
    mpCollisionNode = std::make_unique<CCollisionNode>(mpScene.get(), -1);
    mpCollisionNode->SetCollision(mpCollisionMesh);
    mpUI->Viewport->SetNode(mpCollisionNode.get());

    CCamera& rCamera = mpUI->Viewport->Camera();
    rCamera.SetMoveSpeed(0.5f);
    rCamera.SetPitch(-0.3f);
//    rCamera.SetMoveMode(ECameraMoveMode::Orbit);
//    rCamera.SetOrbit(
}

CCollisionEditorViewport* CCollisionEditor::Viewport() const
{
    return mpUI->Viewport;
}
