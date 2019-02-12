#include "CCollisionEditorViewport.h"

/** Constructor */
CCollisionEditorViewport::CCollisionEditorViewport(QWidget* pParent /*= 0*/)
    : CBasicViewport(pParent)
{
    mpRenderer = std::make_unique<CRenderer>();
    mpRenderer->SetViewportSize(width(), height());
    mpRenderer->SetClearColor(CColor(0.3f, 0.3f, 0.3f));
    mpRenderer->ToggleGrid(true);

    mViewInfo.ShowFlags = EShowFlag::WorldCollision | EShowFlag::ObjectCollision;
    mViewInfo.pRenderer = mpRenderer.get();
    mViewInfo.pScene = nullptr;
    mViewInfo.GameMode = false;
    mViewInfo.CollisionSettings.DrawBoundingHierarchy = true;
}

/** Update the collision node to render in the scene */
void CCollisionEditorViewport::SetNode(CCollisionNode* pNode)
{
    mpCollisionNode = pNode;
}

/** CBasicViewport interface */
void CCollisionEditorViewport::Paint()
{
    mpRenderer->BeginFrame();
    mCamera.LoadMatrices();
    //mGrid.AddToRenderer(mpRenderer.get(), mViewInfo);

    if (mpCollisionNode)
    {
        mpCollisionNode->AddToRenderer(mpRenderer.get(), mViewInfo);
    }

    mpRenderer->RenderBuckets(mViewInfo);
    mpRenderer->EndFrame();
}

void CCollisionEditorViewport::OnResize()
{
    mpRenderer->SetViewportSize(width(), height());
}
