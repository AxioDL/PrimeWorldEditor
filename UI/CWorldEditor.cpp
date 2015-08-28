#include "CWorldEditor.h"
#include "ui_CWorldEditor.h"
#include "CEditorGLWidget.h"
#include <gtc/matrix_transform.hpp>
#include <Core/CDrawUtil.h>
#include <iostream>
#include <QOpenGLContext>
#include <QFontMetrics>
#include <QComboBox>
#include <Core/Log.h>
#include "WDraggableSpinBox.h"
#include "WVectorEditor.h"

#include "WorldEditor/CLayerEditor.h"
#include "WorldEditor/WModifyTab.h"
#include "WorldEditor/WInstancesTab.h"

CWorldEditor::CWorldEditor(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::CWorldEditor)
{
    Log::Write("Creating World Editor");
    ui->setupUi(this);

    mpRenderer = new CRenderer();
    mpRenderer->SetClearColor(CColor::skBlack);
    QSize ViewSize = ui->MainViewport->size();
    mpRenderer->SetViewportSize(ViewSize.width(), ViewSize.height());

    mpSceneManager = new CSceneManager();

    mpArea = nullptr;
    mpWorld = nullptr;
    mpHoverNode = nullptr;
    mDrawSky = true;
    mShowGizmo = false;
    mGizmoHovering = false;
    mGizmoTransforming = false;
    mGizmoUIOutdated = true;

    mFrameCount = 0;
    mFPSTimer.Start();

    // Create blank title bar with some space to allow for dragging the dock
    QWidget *pOldTitleBar = ui->MainDock->titleBarWidget();

    QWidget *pNewTitleBar = new QWidget(ui->MainDock);
    QVBoxLayout *pTitleLayout = new QVBoxLayout(pNewTitleBar);
    pTitleLayout->setSpacing(10);
    pNewTitleBar->setLayout(pTitleLayout);
    ui->MainDock->setTitleBarWidget(pNewTitleBar);

    delete pOldTitleBar;

    // Initialize UI stuff
    ui->ModifyTabContents->SetEditor(this);
    ui->InstancesTabContents->SetEditor(this, mpSceneManager);
    ui->MainDock->installEventFilter(this);
    ui->TransformSpinBox->SetOrientation(Qt::Horizontal);
    ui->TransformSpinBox->layout()->setContentsMargins(0,0,0,0);
    ui->CamSpeedSpinBox->SetDefaultValue(1.0);
    ResetHover();

    mTranslateSpace = eWorldTransform;
    mRotateSpace = eWorldTransform;

    mpTransformSpaceComboBox = new QComboBox(this);
    mpTransformSpaceComboBox->setMinimumWidth(75);
    mpTransformSpaceComboBox->addItem("World");
    mpTransformSpaceComboBox->addItem("Local");
    ui->MainToolBar->insertWidget(0, mpTransformSpaceComboBox);

    // Initialize offscreen actions
    addAction(ui->ActionIncrementGizmo);
    addAction(ui->ActionDecrementGizmo);

    // Connect signals and slots
    connect(ui->TransformSpinBox, SIGNAL(EditingDone(CVector3f)), this, SLOT(OnTransformSpinBoxEdited(CVector3f)));
    connect(ui->TransformSpinBox, SIGNAL(ValueChanged(CVector3f)), this, SLOT(OnTransformSpinBoxModified(CVector3f)));
    connect(mpTransformSpaceComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(SetTransformSpace(int)));
    connect(ui->CamSpeedSpinBox, SIGNAL(valueChanged(double)), this, SLOT(OnCameraSpeedChange(double)));
    connect(ui->MainViewport, SIGNAL(PreRender()), this, SLOT(ViewportPreRender()));
    connect(ui->MainViewport, SIGNAL(Render(CCamera&)), this, SLOT(ViewportRender(CCamera&)));
    connect(ui->MainViewport, SIGNAL(ViewportResized(int,int)), this, SLOT(SetViewportSize(int,int)));
    connect(ui->MainViewport, SIGNAL(frameSwapped()), this, SLOT(ViewportPostRender()));
    connect(ui->MainViewport, SIGNAL(MouseClick(QMouseEvent*)), this, SLOT(ViewportMouseClick(QMouseEvent*)));
    connect(ui->MainViewport, SIGNAL(MouseRelease(QMouseEvent*)), this, SLOT(ViewportMouseRelease(QMouseEvent*)));
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
    ResetHover();
    ClearSelection();
    ui->ModifyTabContents->ClearUI();
    ui->ModifyTabContents->ClearCachedEditors();
    ui->InstancesTabContents->SetMaster(nullptr);
    ui->InstancesTabContents->SetArea(pArea);

    // Clear old area - hack until better world/area loader is implemented
    if ((mpArea) && (pArea != mpArea))
        mpArea->ClearScriptLayers();

    // Load new area
    mpArea = pArea;
    mpWorld = pWorld;
    mAreaToken = CToken(pArea);
    mWorldToken = CToken(pWorld);

    mpSceneManager->SetActiveWorld(pWorld);
    mpSceneManager->SetActiveArea(pArea);

    // Snap camera to location of area
    CTransform4f AreaTransform = pArea->GetTransform();
    CVector3f AreaPosition(AreaTransform[0][3], AreaTransform[1][3], AreaTransform[2][3]);
    ui->MainViewport->Camera().Snap(AreaPosition);

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

void CWorldEditor::ViewportRayCast()
{
    if (!ui->MainViewport->IsMouseInputActive())
    {
        // Cast ray
        CVector2f mouseCoords = ui->MainViewport->MouseDeviceCoordinates();
        CCamera camera = ui->MainViewport->Camera();
        CRay ray = camera.CastRay(mouseCoords);

        if (!mGizmoTransforming)
        {
            // Gizmo hover check
            if (mShowGizmo && !mSelectedNodes.empty())
                mGizmoHovering = mGizmo.CheckSelectedAxes(ray);
            else
            {
                mGizmoHovering = false;
                mGizmo.ResetSelectedAxes();
            }

            // Scene ray check
            SRayIntersection Result = mpSceneManager->SceneRayCast(ray);

            if (Result.Hit)
            {
                if (mpHoverNode)
                    mpHoverNode->SetMouseHovering(false);

                mpHoverNode = Result.pNode;
                mpHoverNode->SetMouseHovering(true);

                mHoverPoint = ray.PointOnRay(Result.Distance);
            }
            else
                ResetHover();
        }
        else
        {
            bool transformed = mGizmo.TransformFromInput(ray, ui->MainViewport->Camera());

            if (transformed)
            {
                switch (mGizmo.Mode())
                {
                    case CGizmo::eTranslate:
                    {
                        CVector3f delta = mGizmo.DeltaTranslation();

                        for (auto it = mSelectedNodes.begin(); it != mSelectedNodes.end(); it++)
                        {
                            (*it)->Translate(delta, mTranslateSpace);
                            (*it)->BuildLightList(this->mpArea);
                        }
                        break;
                    }

                    case CGizmo::eRotate:
                    {
                        CQuaternion delta = mGizmo.DeltaRotation();

                        for (auto it = mSelectedNodes.begin(); it != mSelectedNodes.end(); it++)
                            (*it)->Rotate(delta, mRotateSpace);

                        break;
                    }

                    case CGizmo::eScale:
                    {
                        CVector3f delta = mGizmo.DeltaScale();

                        for (auto it = mSelectedNodes.begin(); it != mSelectedNodes.end(); it++)
                            (*it)->Scale(delta);

                        break;
                    }
                }

                RecalculateSelectionBounds();
                mGizmoUIOutdated = true;
            }
        }
    }
    else
    {
        if (!mGizmoTransforming)
        {
            mGizmoHovering = false;
            mGizmo.ResetSelectedAxes();
        }
        ResetHover();
    }
}

CRenderer* CWorldEditor::Renderer()
{
    return mpRenderer;
}

CSceneManager* CWorldEditor::Scene()
{
    return mpSceneManager;
}

CGameArea* CWorldEditor::ActiveArea()
{
    return mpArea;
}

// ************ SELECTION ************
void CWorldEditor::SelectNode(CSceneNode *pNode)
{
    if (!pNode->IsSelected())
    {
        pNode->SetSelected(true);
        mSelectedNodes.push_back(pNode);
        mSelectionAABox.ExpandBounds(pNode->AABox());

        if (pNode->NodeType() == eScriptNode)
        {
            CScriptNode *pScript = static_cast<CScriptNode*>(pNode);
            if (pScript->HasPreviewVolume())
                mSelectionAABox.ExpandBounds(pScript->PreviewVolumeAABox());
        }
    }

    UpdateSelectionUI();
}

void CWorldEditor::DeselectNode(CSceneNode *pNode)
{
    if (pNode->IsSelected())
    {
        pNode->SetSelected(false);

        for (auto it = mSelectedNodes.begin(); it != mSelectedNodes.end(); it++)
        {
            if (*it == pNode)
            {
                mSelectedNodes.erase(it);
                break;
            }
        }
    }

    RecalculateSelectionBounds();
    UpdateSelectionUI();
}

void CWorldEditor::ClearSelection()
{
    for (auto it = mSelectedNodes.begin(); it != mSelectedNodes.end(); it++)
        (*it)->SetSelected(false);

    mSelectedNodes.clear();
    mSelectionAABox = CAABox::skInfinite;
    UpdateSelectionUI();
}

// ************ SLOTS ************
void CWorldEditor::ViewportMouseDrag(QMouseEvent *pEvent)
{
}

void CWorldEditor::ViewportMouseClick(QMouseEvent *pEvent)
{
    bool AltPressed = ((pEvent->modifiers() & Qt::AltModifier) != 0);
    bool CtrlPressed = ((pEvent->modifiers() & Qt::ControlModifier) != 0);

    if (mGizmoHovering && !AltPressed && !CtrlPressed)
    {
        mGizmoTransforming = true;
        mGizmo.StartTransform();
    }
}

void CWorldEditor::ViewportMouseRelease(QMouseEvent *pEvent)
{
    if (pEvent->button() == Qt::LeftButton)
    {
        // Gizmo transform stop
        if (mGizmoTransforming)
        {
            mGizmo.EndTransform();
            mGizmoTransforming = false;
            mGizmoUIOutdated = true;
        }

        // Object selection/deselection
        else if (!ui->MainViewport->IsMouseInputActive())
        {
            bool ValidNode = ((mpHoverNode) && (mpHoverNode->NodeType() != eStaticNode));
            bool AltPressed = ((pEvent->modifiers() & Qt::AltModifier) != 0);
            bool CtrlPressed = ((pEvent->modifiers() & Qt::ControlModifier) != 0);

            // Alt pressed - deselect object
            if (AltPressed)
            {
                // No valid node selected - do nothing
                if (!ValidNode)
                    return;

                DeselectNode(mpHoverNode);
            }

            // Ctrl pressed - add object to selection
            else if (CtrlPressed)
            {
                // Add hover node to selection
                if (ValidNode)
                    SelectNode(mpHoverNode);
            }

            // Neither pressed
            else
            {
                // If the gizmo isn't under the mouse, clear existing selection + select object (if applicable)
                if (!mGizmoHovering)
                {
                    ClearSelection();

                    if (ValidNode)
                        SelectNode(mpHoverNode);
                }
            }

            UpdateSelectionUI();
        }
    }

    // todo: context menu creation on right-click goes here
}

// ************ SLOTS ************
void CWorldEditor::ViewportPreRender()
{
    // Perform raycast
    if (ui->MainViewport->underMouse())
        ViewportRayCast();
    else
        ResetHover();

    // Start frame
    mFrameTimer.Start();
    mpRenderer->BeginFrame();
}

void CWorldEditor::ViewportRender(CCamera& Camera)
{
    mpSceneManager->AddSceneToRenderer(mpRenderer);

    if (mDrawSky)
    {
        CModel *pSky = mpSceneManager->GetActiveSkybox();
        if (pSky) mpRenderer->RenderSky(pSky, Camera);
    }

    Camera.LoadMatrices();
    mpRenderer->RenderBuckets(Camera);
    mpRenderer->RenderBloom();

    if (mShowGizmo && (mSelectedNodes.size() > 0))
    {
        mpRenderer->ClearDepthBuffer();

        Camera.LoadMatrices();
        mGizmo.UpdateForCamera(Camera);
        mGizmo.AddToRenderer(mpRenderer);

        mpRenderer->RenderBuckets(Camera);
        mpRenderer->ClearDepthBuffer();
    }

    mpRenderer->EndFrame();
    mFrameTimer.Stop();
    mFrameCount++;
}

void CWorldEditor::ViewportPostRender()
{
    // Update UI with raycast results
    UpdateCursor();
    UpdateStatusBar();

    if (mGizmoUIOutdated) UpdateGizmoUI();
}

void CWorldEditor::SetViewportSize(int Width, int Height)
{
    mpRenderer->SetViewportSize(Width, Height);
}

void CWorldEditor::SetTransformSpace(int space)
{
    if (mGizmo.Mode() == CGizmo::eScale) return;
    ETransformSpace& transformSpace = (mGizmo.Mode() == CGizmo::eTranslate ? mTranslateSpace : mRotateSpace);

    switch (space)
    {
    case 0:
        transformSpace = eWorldTransform;
        mGizmo.SetLocalRotation(CQuaternion::skIdentity);
        break;
    case 1:
        transformSpace = eLocalTransform;
        if (!mSelectedNodes.empty())
            mGizmo.SetLocalRotation(mSelectedNodes.front()->AbsoluteRotation());
        break;
    }

    mGizmo.SetTransformSpace(transformSpace);
}

// ************ PRIVATE ************
void CWorldEditor::RecalculateSelectionBounds()
{
    mSelectionAABox = CAABox::skInfinite;

    for (auto it = mSelectedNodes.begin(); it != mSelectedNodes.end(); it++)
    {
        mSelectionAABox.ExpandBounds( (*it)->AABox() );

        if ((*it)->NodeType() == eScriptNode)
        {
            CScriptNode *pScript = static_cast<CScriptNode*>(*it);
            if (pScript->HasPreviewVolume())
                mSelectionAABox.ExpandBounds(pScript->PreviewVolumeAABox());
        }
    }
}

void CWorldEditor::ResetHover()
{
    if (mpHoverNode) mpHoverNode->SetMouseHovering(false);
    mpHoverNode = nullptr;
    mHoverPoint = CVector3f::skZero;
}

void CWorldEditor::UpdateCursor()
{
    if (ui->MainViewport->IsCursorVisible())
    {
        if (mGizmoHovering)
            ui->MainViewport->SetCursorState(Qt::SizeAllCursor);
        else if ((mpHoverNode) && (mpHoverNode->NodeType() != eStaticNode))
            ui->MainViewport->SetCursorState(Qt::PointingHandCursor);
        else
            ui->MainViewport->SetCursorState(Qt::ArrowCursor);
    }
}

void CWorldEditor::UpdateStatusBar()
{
    // Would be cool to do more frequent status bar updates with more info. Unfortunately, this causes lag.
    QString StatusText = "";

    if (!mGizmoHovering && mpHoverNode)
    {
        if (mpHoverNode->NodeType() != eStaticNode)
            StatusText = QString::fromStdString(mpHoverNode->Name());
    }

    if (ui->statusbar->currentMessage() != StatusText)
        ui->statusbar->showMessage(StatusText);
}

void CWorldEditor::UpdateSelectionUI()
{
    // Update sidebar
    ui->ModifyTabContents->GenerateUI(mSelectedNodes);

    // Update selection info text
    QString SelectionText;

    if (mSelectedNodes.size() == 1)
        SelectionText = QString::fromStdString(mSelectedNodes.front()->Name());
    else if (mSelectedNodes.size() > 1)
        SelectionText = QString("%1 objects selected").arg(mSelectedNodes.size());

    QFontMetrics Metrics(ui->SelectionInfoLabel->font());
    SelectionText = Metrics.elidedText(SelectionText, Qt::ElideRight, ui->SelectionInfoFrame->width() - 10);
    ui->SelectionInfoLabel->setText(SelectionText);

    // Update gizmo stuff
    UpdateGizmoUI();
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
                else if (!mSelectedNodes.empty())
                    spinBoxValue = mSelectedNodes.front()->AbsolutePosition();
                break;

            case CGizmo::eRotate:
                if (mGizmoTransforming && mGizmo.HasTransformed())
                    spinBoxValue = mGizmo.TotalRotation();
                else if (!mSelectedNodes.empty())
                    spinBoxValue = mSelectedNodes.front()->AbsoluteRotation().ToEuler();
                break;

            case CGizmo::eScale:
                if (mGizmoTransforming && mGizmo.HasTransformed())
                    spinBoxValue = mGizmo.TotalScale();
                else if (!mSelectedNodes.empty())
                    spinBoxValue = mSelectedNodes.front()->AbsoluteScale();
                break;
            }
        }
        else if (!mSelectedNodes.empty()) spinBoxValue = mSelectedNodes.front()->AbsolutePosition();

        ui->TransformSpinBox->blockSignals(true);
        ui->TransformSpinBox->SetValue(spinBoxValue);
        ui->TransformSpinBox->blockSignals(false);
    }

    // Update gizmo
    if (!mGizmoTransforming)
    {
        if (!mSelectedNodes.empty())
            mGizmo.SetPosition(mSelectedNodes.front()->AbsolutePosition());

        // Determine transform space
        ETransformSpace space;
        if (mGizmo.Mode() == CGizmo::eTranslate)   space = mTranslateSpace;
        else if (mGizmo.Mode() == CGizmo::eRotate) space = mRotateSpace;
        else                                       space = eLocalTransform;

        // Set gizmo transform space
        mGizmo.SetTransformSpace(space);

        if (!mSelectedNodes.empty())
            mGizmo.SetLocalRotation(mSelectedNodes.front()->AbsoluteRotation());
    }

    mGizmoUIOutdated = false;
}

// ************ ACTIONS ************
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
    switch (mGizmo.Mode())
    {
    case CGizmo::eTranslate:
        for (auto it = mSelectedNodes.begin(); it != mSelectedNodes.end(); it++)
            (*it)->SetPosition(value);
        break;

    case CGizmo::eRotate:
        for (auto it = mSelectedNodes.begin(); it != mSelectedNodes.end(); it++)
            (*it)->SetRotation(value);
        break;

    case CGizmo::eScale:
        for (auto it = mSelectedNodes.begin(); it != mSelectedNodes.end(); it++)
            (*it)->SetScale(value);
        break;
    }

    RecalculateSelectionBounds();
    UpdateGizmoUI();
}

void CWorldEditor::OnTransformSpinBoxEdited(CVector3f)
{
    UpdateGizmoUI();
}

// These functions are from "Go to slot" in the designer
void CWorldEditor::on_ActionDrawWorld_triggered()
{
    mpSceneManager->SetWorld(ui->ActionDrawWorld->isChecked());
}

void CWorldEditor::on_ActionDrawCollision_triggered()
{
    mpSceneManager->SetCollision(ui->ActionDrawCollision->isChecked());
}

void CWorldEditor::on_ActionDrawObjects_triggered()
{
    mpSceneManager->SetObjects(ui->ActionDrawObjects->isChecked());
}

void CWorldEditor::on_ActionDrawLights_triggered()
{
    mpSceneManager->SetLights(ui->ActionDrawLights->isChecked());
}

void CWorldEditor::on_ActionDrawSky_triggered()
{
    mDrawSky = ui->ActionDrawSky->isChecked();
}

void CWorldEditor::on_ActionNoLighting_triggered()
{
    CGraphics::sLightMode = CGraphics::NoLighting;
    ui->ActionNoLighting->setChecked(true);
    ui->ActionBasicLighting->setChecked(false);
    ui->ActionWorldLighting->setChecked(false);
}

void CWorldEditor::on_ActionBasicLighting_triggered()
{
    CGraphics::sLightMode = CGraphics::BasicLighting;
    ui->ActionNoLighting->setChecked(false);
    ui->ActionBasicLighting->setChecked(true);
    ui->ActionWorldLighting->setChecked(false);
}

void CWorldEditor::on_ActionWorldLighting_triggered()
{
    CGraphics::sLightMode = CGraphics::WorldLighting;
    ui->ActionNoLighting->setChecked(false);
    ui->ActionBasicLighting->setChecked(false);
    ui->ActionWorldLighting->setChecked(true);
}

void CWorldEditor::on_ActionNoBloom_triggered()
{
    mpRenderer->SetBloom(CRenderer::eNoBloom);
    ui->ActionNoBloom->setChecked(true);
    ui->ActionBloomMaps->setChecked(false);
    ui->ActionFakeBloom->setChecked(false);
    ui->ActionBloom->setChecked(false);
}

void CWorldEditor::on_ActionBloomMaps_triggered()
{
    mpRenderer->SetBloom(CRenderer::eBloomMaps);
    ui->ActionNoBloom->setChecked(false);
    ui->ActionBloomMaps->setChecked(true);
    ui->ActionFakeBloom->setChecked(false);
    ui->ActionBloom->setChecked(false);
}

void CWorldEditor::on_ActionFakeBloom_triggered()
{
    mpRenderer->SetBloom(CRenderer::eFakeBloom);
    ui->ActionNoBloom->setChecked(false);
    ui->ActionBloomMaps->setChecked(false);
    ui->ActionFakeBloom->setChecked(true);
    ui->ActionBloom->setChecked(false);
}

void CWorldEditor::on_ActionBloom_triggered()
{
    mpRenderer->SetBloom(CRenderer::eBloom);
    ui->ActionNoBloom->setChecked(false);
    ui->ActionBloomMaps->setChecked(false);
    ui->ActionFakeBloom->setChecked(false);
    ui->ActionBloom->setChecked(true);
}

void CWorldEditor::on_ActionZoomOnSelection_triggered()
{
    static const float skDistScale = 2.5f;
    static const float skAreaDistScale = 0.8f;

    CCamera& Camera = ui->MainViewport->Camera();
    CVector3f CamDir = Camera.GetDirection();
    CVector3f NewPos;

    // Zoom on selection
    if (mSelectedNodes.size() != 0)
    {
        CVector3f Min = mSelectionAABox.Min();
        CVector3f Max = mSelectionAABox.Max();
        float Dist = ((Max.x - Min.x) + (Max.y - Min.y) + (Max.z - Min.z)) / 3.f;
        //float Dist = mSelectionAABox.Min().Distance(mSelectionAABox.Max());
        NewPos = mSelectionAABox.Center() + (CamDir * -(Dist * skDistScale));
    }

    // Zoom on area
    else
    {
        CAABox AreaBox = mpArea->AABox();
        CVector3f Min = AreaBox.Min();
        CVector3f Max = AreaBox.Max();
        float Dist = ((Max.x - Min.x) + (Max.y - Min.y) + (Max.z - Min.z)) / 3.f;
        //float Dist = AreaBox.Min().Distance(AreaBox.Max());
        NewPos = AreaBox.Center() + (CamDir * -(Dist * skAreaDistScale));
    }

    Camera.SetPosition(NewPos);
}

void CWorldEditor::on_ActionDisableBackfaceCull_triggered()
{
    mpRenderer->ToggleBackfaceCull(!ui->ActionDisableBackfaceCull->isChecked());
}

void CWorldEditor::on_ActionDisableAlpha_triggered()
{
    mpRenderer->ToggleAlphaDisabled(ui->ActionDisableAlpha->isChecked());
}

void CWorldEditor::on_ActionEditLayers_triggered()
{
    // Launch layer editor
    CLayerEditor Editor(this);
    Editor.SetArea(mpArea);
    Editor.exec();
}

void CWorldEditor::on_ActionSelectObjects_triggered()
{
    mShowGizmo = false;
    mGizmoUIOutdated = true;
    ui->ActionSelectObjects->setChecked(true);
    ui->ActionTranslate->setChecked(false);
    ui->ActionRotate->setChecked(false);
    ui->ActionScale->setChecked(false);

    // Set transform spin box settings
    ui->TransformSpinBox->SetSingleStep(0.1);
    ui->TransformSpinBox->SetDefaultValue(0.0);

    // Set transform space combo box
    mpTransformSpaceComboBox->setEnabled(false);
    mpTransformSpaceComboBox->blockSignals(true);
    mpTransformSpaceComboBox->setCurrentIndex(0);
    mpTransformSpaceComboBox->blockSignals(false);
}

void CWorldEditor::on_ActionTranslate_triggered()
{
    mShowGizmo = true;
    mGizmoUIOutdated = true;
    mGizmo.SetMode(CGizmo::eTranslate);
    mGizmo.SetTransformSpace(mTranslateSpace);
    ui->ActionSelectObjects->setChecked(false);
    ui->ActionTranslate->setChecked(true);
    ui->ActionRotate->setChecked(false);
    ui->ActionScale->setChecked(false);

    // Set transform spin box settings
    ui->TransformSpinBox->SetSingleStep(0.1);
    ui->TransformSpinBox->SetDefaultValue(0.0);

    // Set transform space combo box
    int index = (mTranslateSpace == eWorldTransform ? 0 : 1);
    mpTransformSpaceComboBox->setEnabled(true);
    mpTransformSpaceComboBox->blockSignals(true);
    mpTransformSpaceComboBox->setCurrentIndex(index);
    mpTransformSpaceComboBox->blockSignals(false);
}

void CWorldEditor::on_ActionRotate_triggered()
{
    mShowGizmo = true;
    mGizmoUIOutdated = true;
    mGizmo.SetMode(CGizmo::eRotate);
    mGizmo.SetTransformSpace(mRotateSpace);
    ui->ActionSelectObjects->setChecked(false);
    ui->ActionTranslate->setChecked(false);
    ui->ActionRotate->setChecked(true);
    ui->ActionScale->setChecked(false);

    // Set transform spin box settings
    ui->TransformSpinBox->SetSingleStep(1.0);
    ui->TransformSpinBox->SetDefaultValue(0.0);

    // Set transform space combo box
    int index = (mRotateSpace == eWorldTransform ? 0 : 1);
    mpTransformSpaceComboBox->setEnabled(true);
    mpTransformSpaceComboBox->blockSignals(true);
    mpTransformSpaceComboBox->setCurrentIndex(index);
    mpTransformSpaceComboBox->blockSignals(false);
}

void CWorldEditor::on_ActionScale_triggered()
{
    mShowGizmo = true;
    mGizmoUIOutdated = true;
    mGizmo.SetMode(CGizmo::eScale);
    mGizmo.SetTransformSpace(eLocalTransform);
    ui->ActionSelectObjects->setChecked(false);
    ui->ActionTranslate->setChecked(false);
    ui->ActionRotate->setChecked(false);
    ui->ActionScale->setChecked(true);

    // Set transform spin box settings
    ui->TransformSpinBox->SetSingleStep(0.1);
    ui->TransformSpinBox->SetDefaultValue(1.0);

    // Set transform space combo box - force to local
    mpTransformSpaceComboBox->setEnabled(false);
    mpTransformSpaceComboBox->blockSignals(true);
    mpTransformSpaceComboBox->setCurrentIndex(1);
    mpTransformSpaceComboBox->blockSignals(false);
}

void CWorldEditor::on_ActionIncrementGizmo_triggered()
{
    mGizmo.IncrementSize();
}

void CWorldEditor::on_ActionDecrementGizmo_triggered()
{
    mGizmo.DecrementSize();
}
