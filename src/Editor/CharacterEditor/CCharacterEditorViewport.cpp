#include "CCharacterEditorViewport.h"

CCharacterEditorViewport::CCharacterEditorViewport(QWidget *pParent)
    : CBasicViewport(pParent), mpRenderer{std::make_unique<CRenderer>()}
{
    const qreal pixelRatio = devicePixelRatioF();
    mpRenderer->SetViewportSize(width() * pixelRatio, height() * pixelRatio);
    mpRenderer->SetClearColor(CColor(0.3f, 0.3f, 0.3f));
    mpRenderer->ToggleGrid(true);

    mViewInfo.ShowFlags = EShowFlag::ObjectGeometry; // This enables the mesh and not the skeleton by default
    mViewInfo.pRenderer = mpRenderer.get();
    mViewInfo.pScene = nullptr;
    mViewInfo.GameMode = false;
}

CCharacterEditorViewport::~CCharacterEditorViewport() = default;

void CCharacterEditorViewport::SetNode(CCharacterNode *pNode)
{
    mpCharNode = pNode;
}

void CCharacterEditorViewport::CheckUserInput()
{
    uint32 HoverBoneID = UINT32_MAX;

    if (underMouse() && !IsMouseInputActive())
    {
        const CRay Ray = CastRay();
        const SRayIntersection Intersect = mpCharNode->RayNodeIntersectTest(Ray, 0, mViewInfo);

        if (Intersect.Hit)
            HoverBoneID = Intersect.ComponentIndex;
    }

    if (HoverBoneID != mHoverBone)
    {
        mHoverBone = HoverBoneID;
        emit HoverBoneChanged(mHoverBone);
    }
}

void CCharacterEditorViewport::Paint()
{
    mpRenderer->BeginFrame();
    mCamera.LoadMatrices();
    if (mGridEnabled)
        mGrid.AddToRenderer(mpRenderer.get(), mViewInfo);

    if (mpCharNode != nullptr)
    {
        mpCharNode->AddToRenderer(mpRenderer.get(), mViewInfo);
    }

    mpRenderer->RenderBuckets(mViewInfo);
    mpRenderer->EndFrame();
}

void CCharacterEditorViewport::OnResize()
{
    const qreal pixelRatio = devicePixelRatioF();
    mpRenderer->SetViewportSize(width() * pixelRatio, height() * pixelRatio);
}

void CCharacterEditorViewport::OnMouseClick(QMouseEvent *pEvent)
{
    if (pEvent->button() == Qt::LeftButton)
    {
        emit ViewportClick(pEvent);
    }
}
