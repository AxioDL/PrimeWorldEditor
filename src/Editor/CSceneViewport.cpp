#include "Editor/CSceneViewport.h"

#include "Editor/UICommon.h"
#include "Editor/Undo/UndoCommands.h"
#include "Editor/WorldEditor/CWorldEditor.h"
#include <Core/Render/CRenderer.h>
#include <Core/Render/SViewInfo.h>
#include <Core/Resource/Script/CScriptLayer.h>
#include <QApplication>
#include <QMenu>

CSceneViewport::CSceneViewport(QWidget *pParent)
    : CBasicViewport(pParent)
{
    mGrid.SetColor(CColor(0.f, 0.f, 0.6f, 0.f), CColor(0.f, 0.f, 1.f, 0.f));
    mLinkLine.SetColor(CColor::Yellow());

    mpRenderer = std::make_unique<CRenderer>();
    mpRenderer->SetClearColor(CColor::Black());
    const auto pixelRatio = devicePixelRatio();
    mpRenderer->SetViewportSize(width() * pixelRatio, height() * pixelRatio);

    mViewInfo.pScene = mpScene;
    mViewInfo.pRenderer = mpRenderer.get();
    mViewInfo.ShowFlags = EShowFlag::MergedWorld | EShowFlag::ObjectGeometry | EShowFlag::Lights | EShowFlag::Sky;

    CreateContextMenu();
}

CSceneViewport::~CSceneViewport() = default;

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

    if (mViewInfo.ShowFlags.HasFlag(EShowFlag(EShowFlag::SplitWorld | EShowFlag::MergedWorld)))
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
    return mpRenderer.get();
}

CSceneNode* CSceneViewport::HoverNode()
{
    return mpHoverNode;
}

const CVector3f& CSceneViewport::HoverPoint() const
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
        const bool transformed = pGizmo->TransformFromInput(rkRay, mCamera);
        if (transformed)
            emit GizmoMoved();
    }
    else
    {
        mGizmoHovering = false;
    }
}

SRayIntersection CSceneViewport::SceneRayCast(const CRay& rkRay)
{
    if (mpEditor->Gizmo()->IsTransforming())
    {
        ResetHover();
        return {};
    }

    const SRayIntersection Intersect = mpScene->SceneRayCast(rkRay, mViewInfo);
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
    if (mpHoverNode)
        mpHoverNode->SetMouseHovering(false);

    mpHoverNode = nullptr;
}

bool CSceneViewport::IsHoveringGizmo() const
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
    mpToggleSelectAction = new QAction(tr("ToggleSelect"), this);
    connect(mpToggleSelectAction, &QAction::triggered, this, &CSceneViewport::OnToggleSelect);

    mpHideSelectionSeparator = new QAction(this);
    mpHideSelectionSeparator->setSeparator(true);

    mpHideSelectionAction = new QAction(tr("Hide selection"), this);
    connect(mpHideSelectionAction, &QAction::triggered, this, &CSceneViewport::OnHideSelection);

    mpHideUnselectedAction = new QAction(tr("Hide unselected"), this);
    connect(mpHideUnselectedAction, &QAction::triggered, this, &CSceneViewport::OnHideUnselected);

    mpHideHoverSeparator = new QAction(this);
    mpHideHoverSeparator->setSeparator(true);

    mpHideHoverNodeAction = new QAction(tr("HideNode"), this);
    connect(mpHideHoverNodeAction, &QAction::triggered, this, &CSceneViewport::OnHideNode);

    mpHideHoverTypeAction = new QAction(tr("HideType"), this);
    connect(mpHideHoverTypeAction, &QAction::triggered, this, &CSceneViewport::OnHideType);

    mpHideHoverLayerAction = new QAction(tr("HideLayer"), this);
    connect(mpHideHoverLayerAction, &QAction::triggered, this, &CSceneViewport::OnHideLayer);

    mpUnhideSeparator = new QAction(this);
    mpUnhideSeparator->setSeparator(true);

    mpUnhideAllAction = new QAction(tr("Unhide all"), this);
    connect(mpUnhideAllAction, &QAction::triggered, this, &CSceneViewport::OnUnhideAll);

    mpPlayFromHereSeparator = new QAction(this);
    mpPlayFromHereSeparator->setSeparator(true);

    mpPlayFromHereAction = new QAction(tr("Play from here"), this);
    connect(mpPlayFromHereAction, &QAction::triggered, this, &CSceneViewport::OnPlayFromHere);

    const QList<QAction*> Actions{
        mpToggleSelectAction,
        mpHideSelectionSeparator,
        mpHideSelectionAction,
        mpHideUnselectedAction,
        mpHideHoverSeparator,
        mpHideHoverNodeAction,
        mpHideHoverTypeAction,
        mpHideHoverLayerAction,
        mpUnhideSeparator,
        mpUnhideAllAction,
        mpPlayFromHereSeparator,
        mpPlayFromHereAction,
    };
    mpContextMenu->addActions(Actions);

    // Select Connected menu
    mpSelectConnectedMenu = new QMenu(tr("Select connected..."), this);

    mpSelectConnectedOutgoingAction = new QAction(tr("...via outgoing links"), this);
    connect(mpSelectConnectedOutgoingAction, &QAction::triggered, this, &CSceneViewport::OnSelectConnected);

    mpSelectConnectedIncomingAction = new QAction(tr("...via incoming links"), this);
    connect(mpSelectConnectedIncomingAction, &QAction::triggered, this, &CSceneViewport::OnSelectConnected);

    mpSelectConnectedAllAction = new QAction(tr("...via all links"), this);
    connect(mpSelectConnectedAllAction, &QAction::triggered, this, &CSceneViewport::OnSelectConnected);

    mpSelectConnectedMenu->addActions({
        mpSelectConnectedOutgoingAction,
        mpSelectConnectedIncomingAction,
        mpSelectConnectedAllAction,
    });
    mpContextMenu->insertMenu(mpHideSelectionSeparator, mpSelectConnectedMenu);
}

QMouseEvent CSceneViewport::CreateMouseEvent() const
{
    return QMouseEvent(QEvent::MouseMove, mapFromGlobal(QCursor::pos()), QCursor::pos(), Qt::NoButton, qApp->mouseButtons(), qApp->keyboardModifiers());
}

void CSceneViewport::FindConnectedObjects(CInstanceID InstanceID, bool SearchOutgoing, bool SearchIncoming, QList<CInstanceID>& rIDList)
{
    const CScriptNode* pScript = mpScene->NodeForInstanceID(InstanceID);
    if (!pScript)
        return;

    const CScriptObject* pInst = pScript->Instance();
    rIDList.push_back(InstanceID);

    if (SearchOutgoing)
    {
        for (const auto* link : pInst->Links(ELinkType::Outgoing))
        {
            if (!rIDList.contains(link->ReceiverID()))
                FindConnectedObjects(link->ReceiverID(), SearchOutgoing, SearchIncoming, rIDList);
        }
    }

    if (SearchIncoming)
    {
        for (const auto* link : pInst->Links(ELinkType::Incoming))
        {
            if (!rIDList.contains(link->SenderID()))
                FindConnectedObjects(link->SenderID(), SearchOutgoing, SearchIncoming, rIDList);
        }
    }
}

// ************ PROTECTED SLOTS ************
void CSceneViewport::CheckUserInput()
{
    const bool MouseActive = (underMouse() && !IsMouseInputActive());

    if (!MouseActive || mViewInfo.GameMode)
    {
        ResetHover();
        mGizmoHovering = false;
    }

    if (MouseActive)
    {
        const auto ray = CastRay();

        if (!mViewInfo.GameMode)
            CheckGizmoInput(ray);

        if (!mpEditor->Gizmo()->IsTransforming())
            mRayIntersection = SceneRayCast(ray);
    }
    else
    {
        mRayIntersection = {};
    }

    QMouseEvent Event = CreateMouseEvent();
    emit InputProcessed(mRayIntersection, &Event);
}

void CSceneViewport::Paint()
{
    if (!mpScene) return;

    mpRenderer->SetClearColor(CColor::Black());
    mpRenderer->BeginFrame();

    // todo: The sky should really just be a regular node in the background depth group instead of having special rendering code here
    if (mViewInfo.ShowFlags.HasFlag(EShowFlag::Sky) || mViewInfo.GameMode)
    {
        if (CModel* pSky = mpScene->ActiveSkybox())
            mpRenderer->RenderSky(pSky, mViewInfo);
    }

    mCamera.LoadMatrices();
    mpScene->AddSceneToRenderer(mpRenderer.get(), mViewInfo);

    // Add gizmo to renderer
    if (mpEditor->IsGizmoVisible() && !mViewInfo.GameMode)
    {
        CGizmo *pGizmo = mpEditor->Gizmo();
        pGizmo->UpdateForCamera(mCamera);
        pGizmo->AddToRenderer(mpRenderer.get(), mViewInfo);
    }

    // Draw grid if the scene is empty
    if (!mViewInfo.GameMode && mpScene->ActiveArea() == nullptr)
        mGrid.AddToRenderer(mpRenderer.get(), mViewInfo);

    // Draw the line for the link the user is editing.
    if (mLinkLineEnabled)
        mLinkLine.AddToRenderer(mpRenderer.get(), mViewInfo);

    mpRenderer->RenderBuckets(mViewInfo);
    mpRenderer->EndFrame();
}

void CSceneViewport::ContextMenu(QContextMenuEvent *pEvent)
{
    // mpHoverNode is cleared during mouse input, so this call is necessary. todo: better way?
    mRayIntersection = SceneRayCast(CastRay());

    // Set up actions
    const bool HasHoverNode = (mpHoverNode && (mpHoverNode->NodeType() != ENodeType::Static) && (mpHoverNode->NodeType() != ENodeType::Model));
    const bool HasSelection = mpEditor->HasSelection();
    const bool IsScriptNode = (mpHoverNode && mpHoverNode->NodeType() == ENodeType::Script);

    const auto* pOwnerWorldEd = qobject_cast<const CWorldEditor*>(mpEditor);
    const bool QuickplayEnabled = (pOwnerWorldEd && pOwnerWorldEd->IsQuickplayEnabled());

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
            mpToggleSelectAction->setText(tr("Deselect %1").arg(TO_QSTRING(Name)));
        else
            mpToggleSelectAction->setText(tr("Select %1").arg(TO_QSTRING(Name)));
    }

    TString NodeName;
    if (IsScriptNode)
    {
        const auto* pScript = static_cast<CScriptNode*>(mpHoverNode);
        NodeName = pScript->Instance()->InstanceName();
        mpHideHoverTypeAction->setText(tr("Hide all %1 objects").arg(TO_QSTRING(pScript->Template()->Name())));
        mpHideHoverLayerAction->setText(tr("Hide layer %1").arg(TO_QSTRING(pScript->Instance()->Layer()->Name())));
    }
    else if (HasHoverNode)
    {
        NodeName = mpHoverNode->Name();
    }
    mpHideHoverNodeAction->setText(tr("Hide %1").arg(TO_QSTRING(NodeName)));

    // Show menu
    mpMenuNode = mpHoverNode;
    mMenuPoint = mHoverPoint;
    mpContextMenu->exec(pEvent->pos());
}

void CSceneViewport::OnResize()
{
    const auto pixelRatio = devicePixelRatio();
    mpRenderer->SetViewportSize(width() * pixelRatio, height() * pixelRatio);
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
        else // Object selection/deselection
        {
            emit ViewportClick(mRayIntersection, pEvent);
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

void CSceneViewport::OnSelectConnected()
{
    QList<CInstanceID> InstanceIDs;
    bool SearchOutgoing = (sender() == mpSelectConnectedOutgoingAction || sender() == mpSelectConnectedAllAction);
    bool SearchIncoming = (sender() == mpSelectConnectedIncomingAction || sender() == mpSelectConnectedAllAction);
    FindConnectedObjects(static_cast<CScriptNode*>(mpMenuNode)->Instance()->InstanceID(), SearchOutgoing, SearchIncoming, InstanceIDs);

    QList<CSceneNode*> Nodes;
    Nodes.reserve(InstanceIDs.size());
    for (const auto ID : InstanceIDs)
        Nodes.push_back(mpScene->NodeForInstanceID(ID));

    const bool ShouldClear = ((qApp->keyboardModifiers() & Qt::ControlModifier) == 0);
    mpEditor->BatchSelectNodes(std::move(Nodes), ShouldClear, tr("Select Connected"));
}

void CSceneViewport::OnHideSelection()
{
    for (auto* node : mpEditor->Selection()->Nodes())
        node->SetVisible(false);
}

void CSceneViewport::OnHideUnselected()
{
    for (auto* node : mpScene->MakeNodeView(ENodeType::Script | ENodeType::Light))
    {
        if (!node->IsSelected())
            node->SetVisible(false);
    }
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
    for (auto* node : mpScene->MakeNodeView(ENodeType::Script | ENodeType::Light, true))
    {
        if (node->IsVisible())
            continue;

        if (node->NodeType() == ENodeType::Light)
        {
            node->SetVisible(true);
            continue;
        }

        auto* script = static_cast<CScriptNode*>(node);
        if (script->MarkedVisible())
        {
            script->Template()->SetVisible(true);
            script->Instance()->Layer()->SetVisible(true);
        }
        else
        {
            script->SetVisible(true);
        }
    }
}

void CSceneViewport::OnPlayFromHere()
{
    emit PlayFromHere(mpMenuNode, mMenuPoint);
}

void CSceneViewport::OnContextMenuClose()
{
    mpContextMenu = nullptr;
    mpMenuNode = nullptr;
}
