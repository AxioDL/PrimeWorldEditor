#include "CWorldEditor.h"
#include "ui_CWorldEditor.h"
#include "CEditorGLWidget.h"
#include <gtc/matrix_transform.hpp>
#include <Core/CDrawUtil.h>
#include <iostream>
#include <QOpenGLContext>
#include <QFontMetrics>
#include <Core/Log.h>
#include "WDraggableSpinBox.h"

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
    ui->CamSpeedSpinBox->SetDefaultValue(1.0);
    ResetHover();

    // Connect signals and slots
    connect(ui->CamSpeedSpinBox, SIGNAL(valueChanged(double)), this, SLOT(OnCameraSpeedChange(double)));
    connect(ui->MainViewport, SIGNAL(PreRender()), this, SLOT(ViewportPreRender()));
    connect(ui->MainViewport, SIGNAL(Render(CCamera&)), this, SLOT(ViewportRender(CCamera&)));
    connect(ui->MainViewport, SIGNAL(ViewportResized(int,int)), this, SLOT(SetViewportSize(int,int)));
    connect(ui->MainViewport, SIGNAL(frameSwapped()), this, SLOT(ViewportPostRender()));
    connect(ui->MainViewport, SIGNAL(MouseClick(QMouseEvent*)), this, SLOT(ViewportMouseClick(QMouseEvent*)));
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

void CWorldEditor::ViewportRayCast(CRay Ray)
{
    if (!ui->MainViewport->IsMouseInputActive())
    {
        SRayIntersection Result = mpSceneManager->SceneRayCast(Ray);

        if (Result.Hit)
        {
            if (mpHoverNode)
                mpHoverNode->SetMouseHovering(false);

            mpHoverNode = Result.pNode;
            mpHoverNode->SetMouseHovering(true);

            mHoverPoint = Ray.PointOnRay(Result.Distance);
        }
        else
            ResetHover();
    }
    else
        ResetHover();
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
    // todo: gizmo translate/rotate/scale implementation
}

void CWorldEditor::ViewportMouseClick(QMouseEvent *pEvent)
{
    // Process left click (button press)
    if (pEvent->button() == Qt::LeftButton)
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

        // Other - select object
        else
        {
            // Control not pressed - clear existing selection
            if (!CtrlPressed)
                ClearSelection();

            // Add hover node to selection
            if (ValidNode)
                SelectNode(mpHoverNode);
        }

        UpdateSelectionUI();
    }

    // Later, possibly expand to context menu creation for right-click
}

// ************ SLOTS ************
void CWorldEditor::ViewportPreRender()
{
    // Perform raycast
    if (ui->MainViewport->underMouse())
        ViewportRayCast(ui->MainViewport->CastRay());
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
        if (pSky) mpRenderer->RenderSky(pSky, Camera.Position());
    }

    mpRenderer->RenderBuckets(Camera);
    mpRenderer->RenderBloom();
    mpRenderer->EndFrame();
    mFrameTimer.Stop();
    mFrameCount++;
}

void CWorldEditor::ViewportPostRender()
{
    // Update UI with raycast results
    UpdateCursor();
    UpdateStatusBar();
}

void CWorldEditor::SetViewportSize(int Width, int Height)
{
    mpRenderer->SetViewportSize(Width, Height);
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
        if ((mpHoverNode) && (mpHoverNode->NodeType() != eStaticNode))
            ui->MainViewport->SetCursorState(Qt::PointingHandCursor);
        else
            ui->MainViewport->SetCursorState(Qt::ArrowCursor);
    }
}

void CWorldEditor::UpdateStatusBar()
{
    // Would be cool to do more frequent status bar updates with more info. Unfortunately, this causes lag.
    QString StatusText = "";

    if (mpHoverNode)
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

    // Update transform
    CVector3f pos = (!mSelectedNodes.empty() ? mSelectedNodes.front()->GetAbsolutePosition() : CVector3f::skZero);
    ui->XSpinBox->setValue(pos.x);
    ui->YSpinBox->setValue(pos.y);
    ui->ZSpinBox->setValue(pos.z);
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
