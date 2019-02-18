#include "CCollisionEditorViewport.h"

/** Constructor */
CCollisionEditorViewport::CCollisionEditorViewport(QWidget* pParent /*= 0*/)
    : CBasicViewport(pParent)
    , mGridEnabled(true)
{
    mpRenderer = std::make_unique<CRenderer>();
    mpRenderer->SetViewportSize(width(), height());
    mpRenderer->SetClearColor(CColor(0.3f, 0.3f, 0.3f));
    mpRenderer->ToggleGrid(true);

    mViewInfo.ShowFlags = EShowFlag::WorldCollision | EShowFlag::ObjectCollision;
    mViewInfo.pRenderer = mpRenderer.get();
    mViewInfo.pScene = nullptr;
    mViewInfo.GameMode = false;
    mViewInfo.CollisionSettings.DrawBoundingHierarchy = false;
    mViewInfo.CollisionSettings.BoundingHierarchyRenderDepth = 0;
}

/** CBasicViewport interface */
void CCollisionEditorViewport::Paint()
{
    mpRenderer->BeginFrame();
    mCamera.LoadMatrices();
    if (mGridEnabled) mGrid.AddToRenderer(mpRenderer.get(), mViewInfo);

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
