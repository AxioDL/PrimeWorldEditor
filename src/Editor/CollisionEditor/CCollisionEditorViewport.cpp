#include "CCollisionEditorViewport.h"

/** Constructor */
CCollisionEditorViewport::CCollisionEditorViewport(QWidget* pParent)
    : CBasicViewport(pParent)
    , mpRenderer{std::make_unique<CRenderer>()}
{
    const qreal pixelRatio = devicePixelRatioF();
    mpRenderer->SetViewportSize(width() * pixelRatio, height() * pixelRatio);
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
    qreal pixelRatio = devicePixelRatioF();
    mpRenderer->SetViewportSize(width() * pixelRatio, height() * pixelRatio);
}
