#include "CSceneViewport.h"
#include "UICommon.h"
#include "Editor/Undo/UndoCommands.h"
#include <Core/Render/SViewInfo.h>
#include <Core/Resource/Script/CScriptLayer.h>
#include <Core/Scene/CSceneIterator.h>
#include <QMenu>

CSceneViewport::CSceneViewport(QWidget *pParent)
    : CBasicViewport(pParent),
      mpEditor(nullptr),
      mpScene(nullptr),
      mGizmoTransforming(false),
      mpHoverNode(nullptr),
      mHoverPoint(CVector3f::skZero),
      mpContextMenu(nullptr),
      mpMenuNode(nullptr)
{
    mpRenderer = new CRenderer();
    mpRenderer->SetClearColor(CColor::skBlack);
    mpRenderer->SetViewportSize(width(), height());

    mViewInfo.pScene = mpScene;
    mViewInfo.pRenderer = mpRenderer;
    mViewInfo.ShowFlags = eShowWorld | eShowObjectGeometry | eShowLights | eShowSky;

    CreateContextMenu();
}

CSceneViewport::~CSceneViewport()
{
    delete mpRenderer;
}

void CSceneViewport::SetScene(INodeEditor *pEditor, CScene *pScene)
{
    mpEditor = pEditor;
    mpScene = pScene;
}

void CSceneViewport::SetShowFlag(EShowFlag Flag, bool Visible)
{
    if (Visible)
        mViewInfo.ShowFlags |= Flag;
    else
        mViewInfo.ShowFlags &= ~Flag;
}

FShowFlags CSceneViewport::ShowFlags() const
{
    return mViewInfo.ShowFlags;
}

CRenderer* CSceneViewport::Renderer()
{
    return mpRenderer;
}

CSceneNode* CSceneViewport::HoverNode()
{
    return mpHoverNode;
}

CVector3f CSceneViewport::HoverPoint()
{
    return mHoverPoint;
}

void CSceneViewport::CheckGizmoInput(const CRay& ray)
{
    CGizmo *pGizmo = mpEditor->Gizmo();

    // Gizmo not transforming: Check for gizmo hover
    if (!pGizmo->IsTransforming())
    {
        if (mpEditor->IsGizmoVisible())
            mGizmoHovering = pGizmo->CheckSelectedAxes(ray);
        else
            mGizmoHovering = false;
    }

    // Gizmo transforming: Run gizmo input with ray/mouse coords
    else if (mGizmoTransforming)
    {
        bool transformed = pGizmo->TransformFromInput(ray, mCamera);
        if (transformed) emit GizmoMoved();
    }

    else mGizmoHovering = false;
}

void CSceneViewport::SceneRayCast(const CRay& ray)
{
    if (mpEditor->Gizmo()->IsTransforming())
    {
        ResetHover();
        return;
    }

    SRayIntersection result = mpScene->SceneRayCast(ray, mViewInfo);

    if (result.Hit)
    {
        if (mpHoverNode)
            mpHoverNode->SetMouseHovering(false);

        mpHoverNode = result.pNode;
        mpHoverNode->SetMouseHovering(true);
        mHoverPoint = ray.PointOnRay(result.Distance);
    }

    else
        ResetHover();
}

void CSceneViewport::ResetHover()
{
    if (mpHoverNode) mpHoverNode->SetMouseHovering(false);
    mpHoverNode = nullptr;
    mHoverPoint = CVector3f::skZero;
}

bool CSceneViewport::IsHoveringGizmo()
{
    return mGizmoHovering;
}

void CSceneViewport::keyPressEvent(QKeyEvent* pEvent)
{
    CBasicViewport::keyPressEvent(pEvent);

    if (!pEvent->modifiers() && pEvent->key() == Qt::Key_Z  && !pEvent->isAutoRepeat())
    {
        mCamera.SetMoveMode(eOrbitCamera);
        emit CameraOrbit();
    }
}

void CSceneViewport::keyReleaseEvent(QKeyEvent* pEvent)
{
    CBasicViewport::keyReleaseEvent(pEvent);

    if (pEvent->key() == Qt::Key_Z && !pEvent->isAutoRepeat())
    {
        mCamera.SetMoveMode(eFreeCamera);
    }
}

// ************ PROTECTED ************
void CSceneViewport::CreateContextMenu()
{
    mpContextMenu = new QMenu(this);

    mpToggleSelectAction = new QAction("ToggleSelect", this);
    connect(mpToggleSelectAction, SIGNAL(triggered()), this, SLOT(OnToggleSelect()));

    mpHideSelectionSeparator = new QAction(this);
    mpHideSelectionSeparator->setSeparator(true);

    mpHideSelectionAction = new QAction("Hide selection", this);
    connect(mpHideSelectionAction, SIGNAL(triggered()), this, SLOT(OnHideSelection()));

    mpHideUnselectedAction = new QAction("Hide unselected", this);
    connect(mpHideUnselectedAction, SIGNAL(triggered()), this, SLOT(OnHideUnselected()));

    mpHideHoverSeparator = new QAction(this);
    mpHideHoverSeparator->setSeparator(this);

    mpHideHoverNodeAction = new QAction("HideNode", this);
    connect(mpHideHoverNodeAction, SIGNAL(triggered()), this, SLOT(OnHideNode()));

    mpHideHoverTypeAction = new QAction("HideType", this);
    connect(mpHideHoverTypeAction, SIGNAL(triggered()), this, SLOT(OnHideType()));

    mpHideHoverLayerAction = new QAction("HideLayer", this);
    connect(mpHideHoverLayerAction, SIGNAL(triggered()), this, SLOT(OnHideLayer()));

    mpUnhideSeparator = new QAction(this);
    mpUnhideSeparator->setSeparator(true);

    mpUnhideAllAction = new QAction("Unhide all", this);
    connect(mpUnhideAllAction, SIGNAL(triggered()), this, SLOT(OnUnhideAll()));

    QList<QAction*> Actions;
    Actions << mpToggleSelectAction
            << mpHideSelectionSeparator << mpHideSelectionAction << mpHideUnselectedAction
            << mpHideHoverSeparator << mpHideHoverNodeAction << mpHideHoverTypeAction << mpHideHoverLayerAction
            << mpUnhideSeparator << mpUnhideAllAction;

    mpContextMenu->addActions(Actions);
}

// ************ PROTECTED SLOTS ************
void CSceneViewport::CheckUserInput()
{
    bool MouseActive = (underMouse() && !IsMouseInputActive());

    if (!MouseActive || mViewInfo.GameMode)
    {
        ResetHover();
        mGizmoHovering = false;

        if (!MouseActive)
            return;
    }

    CRay ray = CastRay();

    if (!mViewInfo.GameMode)
        CheckGizmoInput(ray);

    if (!mpEditor->Gizmo()->IsTransforming())
        SceneRayCast(ray);
}

void CSceneViewport::Paint()
{
    if (!mpScene) return;

    mpRenderer->BeginFrame();

    if ((mViewInfo.ShowFlags & eShowSky) || mViewInfo.GameMode)
    {
        CModel *pSky = mpScene->GetActiveSkybox();
        if (pSky) mpRenderer->RenderSky(pSky, mViewInfo);
    }

    mCamera.LoadMatrices();
    mpScene->AddSceneToRenderer(mpRenderer, mViewInfo);
    mpRenderer->RenderBuckets(mViewInfo);
    mpRenderer->RenderBloom();

    if (mpEditor->IsGizmoVisible() && !mViewInfo.GameMode)
    {
        CGizmo *pGizmo = mpEditor->Gizmo();
        mCamera.LoadMatrices();

        mpRenderer->ClearDepthBuffer();
        pGizmo->UpdateForCamera(mCamera);
        pGizmo->AddToRenderer(mpRenderer, mViewInfo);
        mpRenderer->RenderBuckets(mViewInfo);
    }

    mpRenderer->EndFrame();
}

void CSceneViewport::ContextMenu(QContextMenuEvent* pEvent)
{
    // mpHoverNode is cleared during mouse input, so this call is necessary. todo: better way?
    SceneRayCast(CastRay());

    // Set up actions
    TString NodeName;
    bool HasHoverNode = (mpHoverNode && mpHoverNode->NodeType() != eStaticNode);
    bool HasSelection = mpEditor->HasSelection();
    bool IsScriptNode = (mpHoverNode && mpHoverNode->NodeType() == eScriptNode);

    mpToggleSelectAction->setVisible(HasHoverNode);
    mpHideSelectionSeparator->setVisible(HasHoverNode);
    mpHideSelectionAction->setVisible(HasSelection);
    mpHideUnselectedAction->setVisible(HasSelection);
    mpHideHoverSeparator->setVisible(HasSelection);
    mpHideHoverNodeAction->setVisible(HasHoverNode);
    mpHideHoverTypeAction->setVisible(IsScriptNode);
    mpHideHoverLayerAction->setVisible(IsScriptNode);
    mpUnhideSeparator->setVisible(HasHoverNode);

    if (HasHoverNode)
    {
        TString Name = IsScriptNode ? static_cast<CScriptNode*>(mpHoverNode)->Object()->InstanceName() : mpHoverNode->Name();

        if (mpHoverNode->IsSelected())
            mpToggleSelectAction->setText(QString("Deselect %1").arg(TO_QSTRING(Name)));
        else
            mpToggleSelectAction->setText(QString("Select %1").arg(TO_QSTRING(Name)));
    }

    if (IsScriptNode)
    {
        CScriptNode *pScript = static_cast<CScriptNode*>(mpHoverNode);
        NodeName = pScript->Object()->InstanceName();
        mpHideHoverTypeAction->setText( QString("Hide all %1 objects").arg(TO_QSTRING(pScript->Template()->TemplateName())) );
        mpHideHoverLayerAction->setText( QString("Hide layer %1").arg(TO_QSTRING(pScript->Object()->Layer()->Name())) );
    }

    else if (HasHoverNode)
        NodeName = mpHoverNode->Name();

    mpHideHoverNodeAction->setText(QString("Hide %1").arg(TO_QSTRING(NodeName)));

    // Show menu
    mpMenuNode = mpHoverNode;
    mpContextMenu->exec(pEvent->pos());
}

void CSceneViewport::OnResize()
{
    mpRenderer->SetViewportSize(width(), height());
}

void CSceneViewport::OnMouseClick(QMouseEvent *pEvent)
{
    bool altPressed = ((pEvent->modifiers() & Qt::AltModifier) != 0);
    bool ctrlPressed = ((pEvent->modifiers() & Qt::ControlModifier) != 0);

    if (mGizmoHovering && !altPressed && !ctrlPressed)
    {
        mGizmoTransforming = true;
        mpEditor->Gizmo()->StartTransform();
        mpEditor->BeginGizmoTransform();
    }
}

void CSceneViewport::OnMouseRelease(QMouseEvent *pEvent)
{
    if (pEvent->button() == Qt::LeftButton)
    {
        // Stop gizmo transform
        if (mGizmoTransforming)
        {
            CGizmo *pGizmo = mpEditor->Gizmo();
            pGizmo->EndTransform();
            mpEditor->EndGizmoTransform();
            mGizmoTransforming = false;
        }

        // Object selection/deselection
        else
        {
            bool validNode = (mpHoverNode && (mpHoverNode->NodeType() != eStaticNode));
            bool altPressed = ((pEvent->modifiers() & Qt::AltModifier) != 0);
            bool ctrlPressed = ((pEvent->modifiers() & Qt::ControlModifier) != 0);

            // Alt: Deselect
            if (altPressed)
            {
                if (!validNode)
                    return;

                mpEditor->DeselectNode(mpHoverNode);
            }

            // Ctrl: Add to selection
            else if (ctrlPressed)
            {
                if (validNode)
                    mpEditor->SelectNode(mpHoverNode);
            }

            // Neither: clear selection + select
            else
            {
                if (!mGizmoHovering)
                {
                    if (validNode)
                        mpEditor->ClearAndSelectNode(mpHoverNode);
                    else
                        mpEditor->ClearSelection();
                }
            }

            mpEditor->UpdateSelectionUI();
        }
    }
}

// ************ MENU ACTIONS ************
void CSceneViewport::OnToggleSelect()
{
    if (mpMenuNode->IsSelected())
        mpEditor->DeselectNode(mpMenuNode);
    else
        mpEditor->SelectNode(mpMenuNode);
}

void CSceneViewport::OnHideSelection()
{
    const QList<CSceneNode*>& rkSelection = mpEditor->GetSelection();

    foreach (CSceneNode *pNode, rkSelection)
        pNode->SetVisible(false);
}

void CSceneViewport::OnHideUnselected()
{
    for (CSceneIterator It(mpScene, eScriptNode | eLightNode); !It.DoneIterating(); ++It)
        if (!It->IsSelected())
            It->SetVisible(false);
}

void CSceneViewport::OnHideNode()
{
    mpMenuNode->SetVisible(false);
}

void CSceneViewport::OnHideType()
{
    static_cast<CScriptNode*>(mpMenuNode)->Template()->SetVisible(false);
}

void CSceneViewport::OnHideLayer()
{
    static_cast<CScriptNode*>(mpMenuNode)->Object()->Layer()->SetVisible(false);
}

void CSceneViewport::OnUnhideAll()
{
    CSceneIterator it(mpScene, eScriptNode | eLightNode, true);

    while (!it.DoneIterating())
    {
        if (!it->IsVisible())
        {
            if (it->NodeType() == eLightNode)
                it->SetVisible(true);

            else
            {
                CScriptNode *pScript = static_cast<CScriptNode*>(*it);

                if (!pScript->MarkedVisible())
                    pScript->SetVisible(true);

                else
                {
                    pScript->Template()->SetVisible(true);
                    pScript->Object()->Layer()->SetVisible(true);
                }
            }
        }

        ++it;
    }
}

void CSceneViewport::OnContextMenuClose()
{
    mpContextMenu = nullptr;
    mpMenuNode = nullptr;
}
