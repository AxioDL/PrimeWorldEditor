#include "CCollisionEditor.h"
#include "ui_CCollisionEditor.h"
#include "Editor/UICommon.h"

#include <QLabel>
#include <QSlider>
#include <QSpacerItem>

CCollisionEditor::CCollisionEditor(CCollisionMeshGroup* pCollisionMesh, QWidget* pParent)
    : IEditor(pParent)
    , mpUI(std::make_unique<Ui::CCollisionEditor>())
{
    mpUI->setupUi(this);

    mpCollisionMesh = pCollisionMesh;

    mpScene = std::make_unique<CScene>();
    mpCollisionNode = std::make_unique<CCollisionNode>(mpScene.get(), -1);
    mpCollisionNode->SetCollision(mpCollisionMesh);
    mpUI->Viewport->SetNode(mpCollisionNode.get());

    CCamera& rCamera = mpUI->Viewport->Camera();
    rCamera.SetMoveSpeed(0.5f);
    rCamera.SetPitch(-0.3f);
    rCamera.SetMoveMode(ECameraMoveMode::Orbit);
    rCamera.SetOrbit(mpCollisionNode->AABox(), 4.f);

    // Add depth widgets to the toolbar
    mpUI->ToolBar->addSeparator();
    mpUI->ToolBar->addWidget(new QLabel(tr("OBBTree: "), this));

    int MaxDepth = 0;
    for (size_t MeshIdx = 0; MeshIdx < pCollisionMesh->NumMeshes(); MeshIdx++)
    {
        const CCollisionMesh* pMesh = pCollisionMesh->MeshByIndex(MeshIdx);
        const int MeshDepth = pMesh->GetRenderData().MaxBoundingHierarchyDepth();
        MaxDepth = std::max(MeshDepth, MaxDepth);
    }

    QSlider* pOBBTreeSlider = new QSlider(this);
    pOBBTreeSlider->setMinimum(0);
    pOBBTreeSlider->setMaximum(MaxDepth);
    pOBBTreeSlider->setOrientation(Qt::Horizontal);
    pOBBTreeSlider->setMaximumWidth(100);

    connect(pOBBTreeSlider, &QSlider::valueChanged,
            this, &CCollisionEditor::OnOBBTreeDepthChanged);

    mpUI->ToolBar->addWidget(pOBBTreeSlider);

    QWidget* pSpacerWidget = new QWidget(this);
    pSpacerWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    mpUI->ToolBar->addWidget(pSpacerWidget);

    // Connections
    connect(mpUI->ActionToggleGrid, &QAction::toggled,
            this, &CCollisionEditor::OnGridToggled);

    connect(mpUI->ActionToggleOrbit, &QAction::toggled,
            this, &CCollisionEditor::OnOrbitToggled);

    // Update window title
    const QString WindowTitle = tr("%APP_FULL_NAME% - Collision Editor - %1[*]")
                                    .arg(TO_QSTRING(mpCollisionMesh->Entry()->CookedAssetPath(true).GetFileName()));
    SET_WINDOWTITLE_APPVARS(WindowTitle);
}

CCollisionEditor::~CCollisionEditor() = default;

CCollisionEditorViewport* CCollisionEditor::Viewport() const
{
    return mpUI->Viewport;
}

void CCollisionEditor::OnGridToggled(bool Enabled)
{
    mpUI->Viewport->SetGridEnabled(Enabled);
}

void CCollisionEditor::OnOrbitToggled(bool Enabled)
{
    CCamera& Camera = mpUI->Viewport->Camera();
    Camera.SetMoveMode( Enabled ? ECameraMoveMode::Orbit : ECameraMoveMode::Free );
}

void CCollisionEditor::OnOBBTreeDepthChanged(int NewValue)
{
    mpUI->Viewport->SetOBBTreeDepth(NewValue);
}
