#include "CCharacterEditorViewport.h"

CCharacterEditorViewport::CCharacterEditorViewport(QWidget *pParent /*= 0*/)
    : CBasicViewport(pParent)
    , mpCharNode(nullptr)
    , mGridEnabled(true)
{
    mpRenderer = new CRenderer();
    qreal pixelRatio = devicePixelRatioF();
    mpRenderer->SetViewportSize(width() * pixelRatio, height() * pixelRatio);
    mpRenderer->SetClearColor(CColor(0.3f, 0.3f, 0.3f));
    mpRenderer->ToggleGrid(true);

    mViewInfo.ShowFlags = EShowFlag::ObjectGeometry; // This enables the mesh and not the skeleton by default
    mViewInfo.pRenderer = mpRenderer;
    mViewInfo.pScene = nullptr;
    mViewInfo.GameMode = false;
}

CCharacterEditorViewport::~CCharacterEditorViewport()
{
    delete mpRenderer;
}

void CCharacterEditorViewport::SetNode(CCharacterNode *pNode)
{
    mpCharNode = pNode;
}

void CCharacterEditorViewport::CheckUserInput()
{
    uint32 HoverBoneID = -1;

    if (underMouse() && !IsMouseInputActive())
    {
        CRay Ray = CastRay();
        SRayIntersection Intersect = mpCharNode->RayNodeIntersectTest(Ray, 0, mViewInfo);

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
    if (mGridEnabled) mGrid.AddToRenderer(mpRenderer, mViewInfo);

    if (mpCharNode)
    {
        mpCharNode->AddToRenderer(mpRenderer, mViewInfo);
    }

    mpRenderer->RenderBuckets(mViewInfo);
    mpRenderer->EndFrame();
}

void CCharacterEditorViewport::OnResize()
{
    qreal pixelRatio = devicePixelRatioF();
    mpRenderer->SetViewportSize(width() * pixelRatio, height() * pixelRatio);
}

void CCharacterEditorViewport::OnMouseClick(QMouseEvent *pEvent)
{
    if (pEvent->button() == Qt::LeftButton)
    {
        emit ViewportClick(pEvent);
    }
}
