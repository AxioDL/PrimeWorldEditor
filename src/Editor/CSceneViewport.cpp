#include "CSceneViewport.h"
#include "CSelectionIterator.h"
#include "UICommon.h"
#include "Editor/Undo/UndoCommands.h"
#include <Core/Render/CDrawUtil.h>
#include <Core/Render/SViewInfo.h>
#include <Core/Resource/Script/CScriptLayer.h>
#include <Core/Scene/CSceneIterator.h>
#include <QApplication>
#include <QMenu>

CSceneViewport::CSceneViewport(QWidget *pParent)
    : CBasicViewport(pParent)
    , mpEditor(nullptr)
    , mpScene(nullptr)
    , mRenderingMergedWorld(true)
    , mGizmoTransforming(false)
    , mpHoverNode(nullptr)
    , mHoverPoint(CVector3f::skZero)
    , mpContextMenu(nullptr)
    , mpMenuNode(nullptr)
{
    mGrid.SetColor(CColor(0.f, 0.f, 0.6f, 0.f), CColor(0.f, 0.f, 1.f, 0.f));
    mLinkLine.SetColor(CColor::skYellow);

    mpRenderer = new CRenderer();
    mpRenderer->SetClearColor(CColor::skBlack);
    mpRenderer->SetViewportSize(width(), height());

    mViewInfo.pScene = mpScene;
    mViewInfo.pRenderer = mpRenderer;
    mViewInfo.ShowFlags = EShowFlag::MergedWorld | EShowFlag::ObjectGeometry | EShowFlag::Lights | EShowFlag::Sky;

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

void CSceneViewport::SetShowWorld(bool Visible)
{
    if (mRenderingMergedWorld)
        SetShowFlag(EShowFlag::MergedWorld, Visible);
    else
        SetShowFlag(EShowFlag::SplitWorld, Visible);
}

void CSceneViewport::SetRenderMergedWorld(bool RenderMerged)
{
    mRenderingMergedWorld = RenderMerged;

    if (mViewInfo.ShowFlags & (EShowFlag::SplitWorld | EShowFlag::MergedWorld))
    {
        SetShowFlag(EShowFlag::SplitWorld, !RenderMerged);
        SetShowFlag(EShowFlag::MergedWorld, RenderMerged);
    }
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

void CSceneViewport::CheckGizmoInput(const CRay& rkRay)
{
    CGizmo *pGizmo = mpEditor->Gizmo();

    // Gizmo not transforming: Check for gizmo hover
    if (!pGizmo->IsTransforming())
    {
        if (mpEditor->IsGizmoVisible())
            mGizmoHovering = pGizmo->CheckSelectedAxes(rkRay);
        else
            mGizmoHovering = false;
    }

    // Gizmo transforming: Run gizmo input with ray/mouse coords
    else if (mGizmoTransforming)
    {
        bool transformed = pGizmo->TransformFromInput(rkRay, mCamera);
        if (transformed) emit GizmoMoved();
    }

    else mGizmoHovering = false;
}

SRayIntersection CSceneViewport::SceneRayCast(const CRay& rkRay)
{
    if (mpEditor->Gizmo()->IsTransforming())
    {
        ResetHover();
        return SRayIntersection();
    }

    SRayIntersection Intersect = mpScene->SceneRayCast(rkRay, mViewInfo);

    if (Intersect.Hit)
    {
        if (mpHoverNode)
            mpHoverNode->SetMouseHovering(false);

        mpHoverNode = Intersect.pNode;
        mpHoverNode->SetMouseHovering(true);
        mHoverPoint = rkRay.PointOnRay(Intersect.Distance);
    }

    else
    {
        mHoverPoint = rkRay.PointOnRay(10.f);
        ResetHover();
    }

    return Intersect;
}

void CSceneViewport::ResetHover()
{
    if (mpHoverNode) mpHoverNode->SetMouseHovering(false);
    mpHoverNode = nullptr;
}

bool CSceneViewport::IsHoveringGizmo()
{
    return mGizmoHovering;
}

void CSceneViewport::keyPressEvent(QKeyEvent *pEvent)
{
    CBasicViewport::keyPressEvent(pEvent);

    if (!pEvent->modifiers() && pEvent->key() == Qt::Key_Z  && !pEvent->isAutoRepeat())
    {
        mCamera.SetMoveMode(ECameraMoveMode::Orbit);
        emit CameraOrbit();
    }
}

void CSceneViewport::keyReleaseEvent(QKeyEvent* pEvent)
{
    CBasicViewport::keyReleaseEvent(pEvent);

    if (pEvent->key() == Qt::Key_Z && !pEvent->isAutoRepeat())
    {
        mCamera.SetMoveMode(ECameraMoveMode::Free);
    }
}

// ************ PROTECTED ************
void CSceneViewport::CreateContextMenu()
{
    mpContextMenu = new QMenu(this);

    // Main context menu
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

    mpPlayFromHereSeparator = new QAction(this);
    mpPlayFromHereSeparator->setSeparator(true);

    mpPlayFromHereAction = new QAction("Play from here");
    connect(mpPlayFromHereAction, SIGNAL(triggered()), this, SLOT(OnPlayFromHere()));

    QList<QAction*> Actions;
    Actions << mpToggleSelectAction
            << mpHideSelectionSeparator << mpHideSelectionAction << mpHideUnselectedAction
            << mpHideHoverSeparator << mpHideHoverNodeAction << mpHideHoverTypeAction << mpHideHoverLayerAction
            << mpUnhideSeparator << mpUnhideAllAction << mpPlayFromHereSeparator << mpPlayFromHereAction;

    mpContextMenu->addActions(Actions);

    // Select Connected menu
    mpSelectConnectedMenu = new QMenu("Select connected...", this);

    mpSelectConnectedOutgoingAction = new QAction("...via outgoing links", this);
    connect(mpSelectConnectedOutgoingAction, SIGNAL(triggered()), this, SLOT(OnSelectConnected()));

    mpSelectConnectedIncomingAction = new QAction("...via incoming links", this);
    connect(mpSelectConnectedIncomingAction, SIGNAL(triggered()), this, SLOT(OnSelectConnected()));

    mpSelectConnectedAllAction = new QAction("...via all links", this);
    connect(mpSelectConnectedAllAction, SIGNAL(triggered()), this, SLOT(OnSelectConnected()));

    QList<QAction*> SelectConnectedActions;
    SelectConnectedActions << mpSelectConnectedOutgoingAction << mpSelectConnectedIncomingAction << mpSelectConnectedAllAction;
    mpSelectConnectedMenu->addActions(SelectConnectedActions);
    mpContextMenu->insertMenu(mpHideSelectionSeparator, mpSelectConnectedMenu);
}

QMouseEvent CSceneViewport::CreateMouseEvent()
{
    return QMouseEvent(QEvent::MouseMove, mapFromGlobal(QCursor::pos()), Qt::NoButton, qApp->mouseButtons(), qApp->keyboardModifiers());
}

void CSceneViewport::FindConnectedObjects(uint32 InstanceID, bool SearchOutgoing, bool SearchIncoming, QList<uint32>& rIDList)
{
    CScriptNode *pScript = mpScene->NodeForInstanceID(InstanceID);
    if (!pScript) return;

    CScriptObject *pInst = pScript->Instance();
    rIDList << InstanceID;

    if (SearchOutgoing)
    {
        for (uint32 iLink = 0; iLink < pInst->NumLinks(ELinkType::Outgoing); iLink++)
        {
            CLink *pLink = pInst->Link(ELinkType::Outgoing, iLink);

            if (!rIDList.contains(pLink->ReceiverID()))
                FindConnectedObjects(pLink->ReceiverID(), SearchOutgoing, SearchIncoming, rIDList);
        }
    }

    if (SearchIncoming)
    {
        for (uint32 iLink = 0; iLink < pInst->NumLinks(ELinkType::Incoming); iLink++)
        {
            CLink *pLink = pInst->Link(ELinkType::Incoming, iLink);

            if (!rIDList.contains(pLink->SenderID()))
                FindConnectedObjects(pLink->SenderID(), SearchOutgoing, SearchIncoming, rIDList);
        }
    }
}

// ************ PROTECTED SLOTS ************
void CSceneViewport::CheckUserInput()
{
    bool MouseActive = (underMouse() && !IsMouseInputActive());

    if (!MouseActive || mViewInfo.GameMode)
    {
        ResetHover();
        mGizmoHovering = false;
    }

    if (MouseActive)
    {
        CRay Ray = CastRay();

        if (!mViewInfo.GameMode)
            CheckGizmoInput(Ray);

        if (!mpEditor->Gizmo()->IsTransforming())
            mRayIntersection = SceneRayCast(Ray);
    }

    else
        mRayIntersection = SRayIntersection();

    QMouseEvent Event = CreateMouseEvent();
    emit InputProcessed(mRayIntersection, &Event);
}

void CSceneViewport::Paint()
{
    if (!mpScene) return;

    mpRenderer->SetClearColor(CColor::skBlack);
    mpRenderer->BeginFrame();

    // todo: The sky should really just be a regular node in the background depth group instead of having special rendering code here
    if ((mViewInfo.ShowFlags & EShowFlag::Sky) || mViewInfo.GameMode)
    {
        CModel *pSky = mpScene->ActiveSkybox();
        if (pSky) mpRenderer->RenderSky(pSky, mViewInfo);
    }

    mCamera.LoadMatrices();
    mpScene->AddSceneToRenderer(mpRenderer, mViewInfo);

    // Add gizmo to renderer
    if (mpEditor->IsGizmoVisible() && !mViewInfo.GameMode)
    {
        CGizmo *pGizmo = mpEditor->Gizmo();
        pGizmo->UpdateForCamera(mCamera);
        pGizmo->AddToRenderer(mpRenderer, mViewInfo);
    }

    // Draw grid if the scene is empty
    if (!mViewInfo.GameMode && mpScene->ActiveArea() == nullptr)
        mGrid.AddToRenderer(mpRenderer, mViewInfo);

    // Draw the line for the link the user is editing.
    if (mLinkLineEnabled) mLinkLine.AddToRenderer(mpRenderer, mViewInfo);

    mpRenderer->RenderBuckets(mViewInfo);
    mpRenderer->EndFrame();
}

void CSceneViewport::ContextMenu(QContextMenuEvent *pEvent)
{
    // mpHoverNode is cleared during mouse input, so this call is necessary. todo: better way?
    mRayIntersection = SceneRayCast(CastRay());

    // Set up actions
    TString NodeName;
    bool HasHoverNode = (mpHoverNode && (mpHoverNode->NodeType() != ENodeType::Static) && (mpHoverNode->NodeType() != ENodeType::Model));
    bool HasSelection = mpEditor->HasSelection();
    bool IsScriptNode = (mpHoverNode && mpHoverNode->NodeType() == ENodeType::Script);

    CWorldEditor* pOwnerWorldEd = qobject_cast<CWorldEditor*>(mpEditor);
    bool QuickplayEnabled = (pOwnerWorldEd && pOwnerWorldEd->IsQuickplayEnabled());

    mpToggleSelectAction->setVisible(HasHoverNode);
    mpSelectConnectedMenu->menuAction()->setVisible(IsScriptNode);
    mpHideSelectionSeparator->setVisible(HasHoverNode);
    mpHideSelectionAction->setVisible(HasSelection);
    mpHideUnselectedAction->setVisible(HasSelection);
    mpHideHoverSeparator->setVisible(HasSelection);
    mpHideHoverNodeAction->setVisible(HasHoverNode);
    mpHideHoverTypeAction->setVisible(IsScriptNode);
    mpHideHoverLayerAction->setVisible(IsScriptNode);
    mpUnhideSeparator->setVisible(HasHoverNode);
    mpPlayFromHereSeparator->setVisible(QuickplayEnabled);
    mpPlayFromHereAction->setVisible(QuickplayEnabled);

    if (HasHoverNode)
    {
        TString Name = IsScriptNode ? static_cast<CScriptNode*>(mpHoverNode)->Instance()->InstanceName() : mpHoverNode->Name();

        if (mpHoverNode->IsSelected())
            mpToggleSelectAction->setText(QString("Deselect %1").arg(TO_QSTRING(Name)));
        else
            mpToggleSelectAction->setText(QString("Select %1").arg(TO_QSTRING(Name)));
    }

    if (IsScriptNode)
    {
        CScriptNode *pScript = static_cast<CScriptNode*>(mpHoverNode);
        NodeName = pScript->Instance()->InstanceName();
        mpHideHoverTypeAction->setText( QString("Hide all %1 objects").arg(TO_QSTRING(pScript->Template()->Name())) );
        mpHideHoverLayerAction->setText( QString("Hide layer %1").arg(TO_QSTRING(pScript->Instance()->Layer()->Name())) );
    }

    else if (HasHoverNode)
        NodeName = mpHoverNode->Name();

    mpHideHoverNodeAction->setText(QString("Hide %1").arg(TO_QSTRING(NodeName)));

    // Show menu
    mpMenuNode = mpHoverNode;
    mMenuPoint = mHoverPoint;
    mpContextMenu->exec(pEvent->pos());
}

void CSceneViewport::OnResize()
{
    mpRenderer->SetViewportSize(width(), height());
}

void CSceneViewport::OnMouseClick(QMouseEvent *pEvent)
{
    bool AltPressed = ((pEvent->modifiers() & Qt::AltModifier) != 0);
    bool CtrlPressed = ((pEvent->modifiers() & Qt::ControlModifier) != 0);

    if (mGizmoHovering && !AltPressed && !CtrlPressed)
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
            emit ViewportClick(mRayIntersection, pEvent);
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

void CSceneViewport::OnSelectConnected()
{
    QList<uint32> InstanceIDs;
    bool SearchOutgoing = (sender() == mpSelectConnectedOutgoingAction || sender() == mpSelectConnectedAllAction);
    bool SearchIncoming = (sender() == mpSelectConnectedIncomingAction || sender() == mpSelectConnectedAllAction);
    FindConnectedObjects(static_cast<CScriptNode*>(mpMenuNode)->Instance()->InstanceID(), SearchOutgoing, SearchIncoming, InstanceIDs);

    QList<CSceneNode*> Nodes;
    foreach (uint32 ID, InstanceIDs)
        Nodes << mpScene->NodeForInstanceID(ID);

    bool ShouldClear = ((qApp->keyboardModifiers() & Qt::ControlModifier) == 0);
    mpEditor->BatchSelectNodes(Nodes, ShouldClear, "Select Connected");
}

void CSceneViewport::OnHideSelection()
{
    for (CSelectionIterator It(mpEditor->Selection()); It; ++It)
        It->SetVisible(false);
}

void CSceneViewport::OnHideUnselected()
{
    for (CSceneIterator It(mpScene, ENodeType::Script | ENodeType::Light); !It.DoneIterating(); ++It)
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
    static_cast<CScriptNode*>(mpMenuNode)->Instance()->Layer()->SetVisible(false);
}

void CSceneViewport::OnUnhideAll()
{
    CSceneIterator it(mpScene, ENodeType::Script | ENodeType::Light, true);

    while (!it.DoneIterating())
    {
        if (!it->IsVisible())
        {
            if (it->NodeType() == ENodeType::Light)
                it->SetVisible(true);

            else
            {
                CScriptNode *pScript = static_cast<CScriptNode*>(*it);

                if (!pScript->MarkedVisible())
                    pScript->SetVisible(true);

                else
                {
                    pScript->Template()->SetVisible(true);
                    pScript->Instance()->Layer()->SetVisible(true);
                }
            }
        }

        ++it;
    }
}

void CSceneViewport::OnPlayFromHere()
{
    CWorldEditor* pOwnerWorldEd = qobject_cast<CWorldEditor*>(mpEditor);
    ASSERT( pOwnerWorldEd != nullptr );

    if (mpMenuNode)
    {
        pOwnerWorldEd->LaunchQuickplayFromLocation(mMenuPoint, true);
    }
    else
    {
        pOwnerWorldEd->LaunchQuickplay();
    }
}

void CSceneViewport::OnContextMenuClose()
{
    mpContextMenu = nullptr;
    mpMenuNode = nullptr;
}
