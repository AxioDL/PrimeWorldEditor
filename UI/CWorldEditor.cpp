#include "CWorldEditor.h"
#include "ui_CWorldEditor.h"
#include "CBasicViewport.h"
#include <gtc/matrix_transform.hpp>
#include <Core/CDrawUtil.h>
#include <iostream>
#include <QOpenGLContext>
#include <QFontMetrics>
#include <QComboBox>
#include <Core/Log.h>
#include "WDraggableSpinBox.h"
#include "WVectorEditor.h"
#include "undo/UndoCommands.h"
#include "UICommon.h"

#include "WorldEditor/CLayerEditor.h"
#include "WorldEditor/WModifyTab.h"
#include "WorldEditor/WInstancesTab.h"

CWorldEditor::CWorldEditor(QWidget *parent) :
    INodeEditor(parent),
    ui(new Ui::CWorldEditor)
{
    Log::Write("Creating World Editor");
    ui->setupUi(this);

    mpArea = nullptr;
    mpWorld = nullptr;
    mGizmoHovering = false;
    mGizmoTransforming = false;

    // Start refresh timer
    connect(&mRefreshTimer, SIGNAL(timeout()), this, SLOT(RefreshViewport()));
    mRefreshTimer.start(0);

    // Create blank title bar with some space to allow for dragging the dock
    QWidget *pOldTitleBar = ui->MainDock->titleBarWidget();

    QWidget *pNewTitleBar = new QWidget(ui->MainDock);
    QVBoxLayout *pTitleLayout = new QVBoxLayout(pNewTitleBar);
    pTitleLayout->setSpacing(10);
    pNewTitleBar->setLayout(pTitleLayout);
    ui->MainDock->setTitleBarWidget(pNewTitleBar);

    delete pOldTitleBar;

    // Initialize UI stuff
    ui->MainViewport->SetScene(this, &mScene);
    ui->ModifyTabContents->SetEditor(this);
    ui->InstancesTabContents->SetEditor(this, &mScene);
    ui->MainDock->installEventFilter(this);
    ui->TransformSpinBox->SetOrientation(Qt::Horizontal);
    ui->TransformSpinBox->layout()->setContentsMargins(0,0,0,0);
    ui->CamSpeedSpinBox->SetDefaultValue(1.0);

    mpTransformCombo->setMinimumWidth(75);
    ui->MainToolBar->addActions(mGizmoActions);
    ui->MainToolBar->addWidget(mpTransformCombo);
    ui->menuEdit->addActions(mUndoActions);

    // Initialize offscreen actions
    addAction(ui->ActionIncrementGizmo);
    addAction(ui->ActionDecrementGizmo);

    // Connect signals and slots
    connect(ui->MainViewport, SIGNAL(GizmoMoved()), this, SLOT(OnGizmoMoved()));
    connect(ui->MainViewport, SIGNAL(CameraOrbit()), this, SLOT(UpdateCameraOrbit()));
    connect(this, SIGNAL(SelectionModified()), this, SLOT(UpdateCameraOrbit()));
    connect(this, SIGNAL(SelectionTransformed()), this, SLOT(UpdateCameraOrbit()));
    connect(ui->TransformSpinBox, SIGNAL(ValueChanged(CVector3f)), this, SLOT(OnTransformSpinBoxModified(CVector3f)));
    connect(ui->TransformSpinBox, SIGNAL(EditingDone(CVector3f)), this, SLOT(OnTransformSpinBoxEdited(CVector3f)));
    connect(ui->CamSpeedSpinBox, SIGNAL(valueChanged(double)), this, SLOT(OnCameraSpeedChange(double)));
}

CWorldEditor::~CWorldEditor()
{
    delete ui;
}

bool CWorldEditor::eventFilter(QObject *pObj, QEvent *pEvent)
{
    if (pObj == ui->MainDock)
    {
        if (pEvent->type() == QEvent::Resize)
        {
            UpdateSelectionUI();
        }
    }

    return false;
}

void CWorldEditor::SetArea(CWorld *pWorld, CGameArea *pArea)
{
    ui->MainViewport->ResetHover();
    ClearSelection();
    ui->ModifyTabContents->ClearUI();
    ui->ModifyTabContents->ClearCachedEditors();
    ui->InstancesTabContents->SetMaster(nullptr);
    ui->InstancesTabContents->SetArea(pArea);
    mUndoStack.clear();

    // Clear old area - hack until better world/area loader is implemented
    if ((mpArea) && (pArea != mpArea))
        mpArea->ClearScriptLayers();

    // Load new area
    mpArea = pArea;
    mpWorld = pWorld;
    mScene.SetActiveWorld(pWorld);
    mScene.SetActiveArea(pArea);

    // Snap camera to new area
    CCamera *pCamera = &ui->MainViewport->Camera();

    if (pCamera->MoveMode() == eFreeCamera)
    {
        CTransform4f AreaTransform = pArea->GetTransform();
        CVector3f AreaPosition(AreaTransform[0][3], AreaTransform[1][3], AreaTransform[2][3]);
        pCamera->Snap(AreaPosition);
    }

    UpdateCameraOrbit();

    // Default bloom to Fake Bloom for Metroid Prime 3; disable for other games
    if (mpWorld->Version() == eCorruption)
    {
        ui->menuBloom->setEnabled(true);
        on_ActionFakeBloom_triggered();
    }

    else
    {
        ui->menuBloom->setEnabled(false);
        on_ActionNoBloom_triggered();
    }

    // Set up sidebar tabs
    CMasterTemplate *pMaster = CMasterTemplate::GetMasterForGame(mpWorld->Version());
    ui->InstancesTabContents->SetMaster(pMaster);
}

CGameArea* CWorldEditor::ActiveArea()
{
    return mpArea;
}

// ************ UPDATE UI ************
void CWorldEditor::UpdateStatusBar()
{
    // Would be cool to do more frequent status bar updates with more info. Unfortunately, this causes lag.
    QString StatusText = "";

    if (!mGizmoHovering)
    {
        if (ui->MainViewport->underMouse())
        {
            CSceneNode *pHoverNode = ui->MainViewport->HoverNode();

            if (pHoverNode && (pHoverNode->NodeType() != eStaticNode))
                StatusText = TO_QSTRING(pHoverNode->Name());
        }
    }

    if (ui->statusbar->currentMessage() != StatusText)
        ui->statusbar->showMessage(StatusText);
}

void CWorldEditor::UpdateGizmoUI()
{
    // Update transform XYZ spin boxes
    if (!ui->TransformSpinBox->IsBeingDragged())
    {
        CVector3f spinBoxValue = CVector3f::skZero;

        // If the gizmo is transforming, use the total transform amount
        // Otherwise, use the first selected node transform, or 0 if no selection
        if (mShowGizmo)
        {
            switch (mGizmo.Mode())
            {
            case CGizmo::eTranslate:
                if (mGizmoTransforming && mGizmo.HasTransformed())
                    spinBoxValue = mGizmo.TotalTranslation();
                else if (!mSelection.empty())
                    spinBoxValue = mSelection.front()->AbsolutePosition();
                break;

            case CGizmo::eRotate:
                if (mGizmoTransforming && mGizmo.HasTransformed())
                    spinBoxValue = mGizmo.TotalRotation();
                else if (!mSelection.empty())
                    spinBoxValue = mSelection.front()->AbsoluteRotation().ToEuler();
                break;

            case CGizmo::eScale:
                if (mGizmoTransforming && mGizmo.HasTransformed())
                    spinBoxValue = mGizmo.TotalScale();
                else if (!mSelection.empty())
                    spinBoxValue = mSelection.front()->AbsoluteScale();
                break;
            }
        }
        else if (!mSelection.empty()) spinBoxValue = mSelection.front()->AbsolutePosition();

        ui->TransformSpinBox->blockSignals(true);
        ui->TransformSpinBox->SetValue(spinBoxValue);
        ui->TransformSpinBox->blockSignals(false);
    }

    // Update gizmo
    if (!mGizmoTransforming)
    {
        // Set gizmo transform
        if (!mSelection.empty())
        {
            mGizmo.SetPosition(mSelection.front()->AbsolutePosition());
            mGizmo.SetLocalRotation(mSelection.front()->AbsoluteRotation());
        }
    }
}

void CWorldEditor::UpdateSelectionUI()
{
    // Update sidebar
    ui->ModifyTabContents->GenerateUI(mSelection);

    // Update selection info text
    QString SelectionText;

    if (mSelection.size() == 1)
        SelectionText = TO_QSTRING(mSelection.front()->Name());
    else if (mSelection.size() > 1)
        SelectionText = QString("%1 objects selected").arg(mSelection.size());

    QFontMetrics Metrics(ui->SelectionInfoLabel->font());
    SelectionText = Metrics.elidedText(SelectionText, Qt::ElideRight, ui->SelectionInfoFrame->width() - 10);
    ui->SelectionInfoLabel->setText(SelectionText);

    // Update gizmo stuff
    UpdateGizmoUI();
}

// ************ PROTECTED ************
void CWorldEditor::GizmoModeChanged(CGizmo::EGizmoMode mode)
{
    ui->TransformSpinBox->SetSingleStep( (mode == CGizmo::eRotate ? 1.0 : 0.1) );
    ui->TransformSpinBox->SetDefaultValue( (mode == CGizmo::eScale ? 1.0 : 0.0) );
}

void CWorldEditor::UpdateCursor()
{
    if (ui->MainViewport->IsCursorVisible())
    {
        CSceneNode *pHoverNode = ui->MainViewport->HoverNode();

        if (ui->MainViewport->IsHoveringGizmo())
            ui->MainViewport->SetCursorState(Qt::SizeAllCursor);
        else if ((pHoverNode) && (pHoverNode->NodeType() != eStaticNode))
            ui->MainViewport->SetCursorState(Qt::PointingHandCursor);
        else
            ui->MainViewport->SetCursorState(Qt::ArrowCursor);
    }
}

// ************ PRIVATE SLOTS ************
void CWorldEditor::RefreshViewport()
{
    if (!mGizmo.IsTransforming())
        mGizmo.ResetSelectedAxes();

    // Process input + update UI
    ui->MainViewport->ProcessInput();
    UpdateCursor();
    UpdateStatusBar();
    UpdateGizmoUI();

    // Render
    ui->MainViewport->Render();
}

void CWorldEditor::UpdateCameraOrbit()
{
    CCamera *pCamera = &ui->MainViewport->Camera();

    if (!mSelection.isEmpty())
        pCamera->SetOrbit(mSelectionBounds);
    else if (mpArea)
        pCamera->SetOrbit(mpArea->AABox(), 1.5f);
}

void CWorldEditor::OnCameraSpeedChange(double speed)
{
    static const double skDefaultSpeed = 1.0;
    ui->MainViewport->Camera().SetMoveSpeed(skDefaultSpeed * speed);

    ui->CamSpeedSpinBox->blockSignals(true);
    ui->CamSpeedSpinBox->setValue(speed);
    ui->CamSpeedSpinBox->blockSignals(false);
}

void CWorldEditor::OnTransformSpinBoxModified(CVector3f value)
{
    if (mSelection.empty()) return;

    switch (mGizmo.Mode())
    {
        // Use absolute position/rotation, but relative scale. (This way spinbox doesn't show preview multiplier)
        case CGizmo::eTranslate:
        {
            CVector3f delta = value - mSelection.front()->AbsolutePosition();
            mUndoStack.push(new CTranslateNodeCommand(this, mSelection, delta, mTranslateSpace));
            break;
        }

        case CGizmo::eRotate:
        {
            CQuaternion delta = CQuaternion::FromEuler(value) * mSelection.front()->AbsoluteRotation().Inverse();
            mUndoStack.push(new CRotateNodeCommand(this, mSelection, CVector3f::skZero, delta, mRotateSpace));
            break;
        }

        case CGizmo::eScale:
        {
            CVector3f delta = value / mSelection.front()->AbsoluteScale();
            mUndoStack.push(new CScaleNodeCommand(this, mSelection, CVector3f::skZero, delta));
            break;
        }
    }

    RecalculateSelectionBounds();
    UpdateGizmoUI();
}

void CWorldEditor::OnTransformSpinBoxEdited(CVector3f)
{
    // bit of a hack - the vector editor emits a second "editing done" signal when it loses focus
    ui->TransformSpinBox->blockSignals(true);
    ui->MainViewport->setFocus();
    ui->TransformSpinBox->blockSignals(false);
    if (mSelection.empty()) return;

    if (mGizmo.Mode() == CGizmo::eTranslate)   mUndoStack.push(CTranslateNodeCommand::End());
    else if (mGizmo.Mode() == CGizmo::eRotate) mUndoStack.push(CRotateNodeCommand::End());
    else if (mGizmo.Mode() == CGizmo::eScale)  mUndoStack.push(CScaleNodeCommand::End());

    UpdateGizmoUI();
}

// These functions are from "Go to slot" in the designer
void CWorldEditor::on_ActionDrawWorld_triggered()
{
    ui->MainViewport->Renderer()->ToggleWorld(ui->ActionDrawWorld->isChecked());
}

void CWorldEditor::on_ActionDrawCollision_triggered()
{
    ui->MainViewport->Renderer()->ToggleWorldCollision(ui->ActionDrawCollision->isChecked());
}

void CWorldEditor::on_ActionDrawObjects_triggered()
{
    ui->MainViewport->Renderer()->ToggleObjects(ui->ActionDrawObjects->isChecked());
}

void CWorldEditor::on_ActionDrawLights_triggered()
{
    ui->MainViewport->Renderer()->ToggleLights(ui->ActionDrawLights->isChecked());
}

void CWorldEditor::on_ActionDrawSky_triggered()
{
    ui->MainViewport->SetSkyEnabled(ui->ActionDrawSky->isChecked());
}

void CWorldEditor::on_ActionNoLighting_triggered()
{
    CGraphics::sLightMode = CGraphics::eNoLighting;
    ui->ActionNoLighting->setChecked(true);
    ui->ActionBasicLighting->setChecked(false);
    ui->ActionWorldLighting->setChecked(false);
}

void CWorldEditor::on_ActionBasicLighting_triggered()
{
    CGraphics::sLightMode = CGraphics::eBasicLighting;
    ui->ActionNoLighting->setChecked(false);
    ui->ActionBasicLighting->setChecked(true);
    ui->ActionWorldLighting->setChecked(false);
}

void CWorldEditor::on_ActionWorldLighting_triggered()
{
    CGraphics::sLightMode = CGraphics::eWorldLighting;
    ui->ActionNoLighting->setChecked(false);
    ui->ActionBasicLighting->setChecked(false);
    ui->ActionWorldLighting->setChecked(true);
}

void CWorldEditor::on_ActionNoBloom_triggered()
{
    ui->MainViewport->Renderer()->SetBloom(CRenderer::eNoBloom);
    ui->ActionNoBloom->setChecked(true);
    ui->ActionBloomMaps->setChecked(false);
    ui->ActionFakeBloom->setChecked(false);
    ui->ActionBloom->setChecked(false);
}

void CWorldEditor::on_ActionBloomMaps_triggered()
{
    ui->MainViewport->Renderer()->SetBloom(CRenderer::eBloomMaps);
    ui->ActionNoBloom->setChecked(false);
    ui->ActionBloomMaps->setChecked(true);
    ui->ActionFakeBloom->setChecked(false);
    ui->ActionBloom->setChecked(false);
}

void CWorldEditor::on_ActionFakeBloom_triggered()
{
    ui->MainViewport->Renderer()->SetBloom(CRenderer::eFakeBloom);
    ui->ActionNoBloom->setChecked(false);
    ui->ActionBloomMaps->setChecked(false);
    ui->ActionFakeBloom->setChecked(true);
    ui->ActionBloom->setChecked(false);
}

void CWorldEditor::on_ActionBloom_triggered()
{
    ui->MainViewport->Renderer()->SetBloom(CRenderer::eBloom);
    ui->ActionNoBloom->setChecked(false);
    ui->ActionBloomMaps->setChecked(false);
    ui->ActionFakeBloom->setChecked(false);
    ui->ActionBloom->setChecked(true);
}

void CWorldEditor::on_ActionDisableBackfaceCull_triggered()
{
    ui->MainViewport->Renderer()->ToggleBackfaceCull(!ui->ActionDisableBackfaceCull->isChecked());
}

void CWorldEditor::on_ActionDisableAlpha_triggered()
{
    ui->MainViewport->Renderer()->ToggleAlphaDisabled(ui->ActionDisableAlpha->isChecked());
}

void CWorldEditor::on_ActionEditLayers_triggered()
{
    // Launch layer editor
    CLayerEditor Editor(this);
    Editor.SetArea(mpArea);
    Editor.exec();
}

void CWorldEditor::on_ActionIncrementGizmo_triggered()
{
    mGizmo.IncrementSize();
}

void CWorldEditor::on_ActionDecrementGizmo_triggered()
{
    mGizmo.DecrementSize();
}

void CWorldEditor::on_ActionDrawObjectCollision_triggered()
{
    ui->MainViewport->Renderer()->ToggleObjectCollision(ui->ActionDrawObjectCollision->isChecked());
}

void CWorldEditor::on_ActionGameMode_triggered()
{
    ui->MainViewport->SetGameMode(ui->ActionGameMode->isChecked());
}
