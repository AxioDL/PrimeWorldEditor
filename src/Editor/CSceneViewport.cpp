#include "CSceneViewport.h"
#include "undo/UndoCommands.h"
#include <Core/SViewInfo.h>

CSceneViewport::CSceneViewport(QWidget *pParent)
    : CBasicViewport(pParent),
      mpEditor(nullptr),
      mpScene(nullptr),
      mDrawSky(true),
      mGizmoTransforming(false),
      mpHoverNode(nullptr),
      mHoverPoint(CVector3f::skZero)
{
    mpRenderer = new CRenderer();
    mpRenderer->SetClearColor(CColor::skBlack);
    mpRenderer->SetViewportSize(width(), height());

    mViewInfo.pScene = mpScene;
    mViewInfo.pRenderer = mpRenderer;
}

CSceneViewport::~CSceneViewport()
{
    delete mpRenderer;
}

void CSceneViewport::SetScene(INodeEditor *pEditor, CSceneManager *pScene)
{
    mpEditor = pEditor;
    mpScene = pScene;
}

void CSceneViewport::SetSkyEnabled(bool b)
{
    mDrawSky = b;
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

    if (mDrawSky || mViewInfo.GameMode)
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

void CSceneViewport::ContextMenu(QContextMenuEvent* /*pEvent*/)
{
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
