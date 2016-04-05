#include "CCharacterEditorViewport.h"

CCharacterEditorViewport::CCharacterEditorViewport(QWidget *pParent /*= 0*/)
    : CBasicViewport(pParent)
    , mpCharNode(nullptr)
{
    mpRenderer = new CRenderer();
    mpRenderer->SetViewportSize(width(), height());
    mpRenderer->SetClearColor(CColor(0.3f, 0.3f, 0.3f));
    mpRenderer->ToggleGrid(true);

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

void CCharacterEditorViewport::Paint()
{
    mpRenderer->BeginFrame();
    mCamera.LoadMatrices();
    CDrawUtil::DrawGrid();

    if (mpCharNode)
    {
        mpCharNode->AddToRenderer(mpRenderer, mViewInfo);
        mpRenderer->RenderBuckets(mViewInfo);
    }

    mpRenderer->EndFrame();
}

void CCharacterEditorViewport::OnResize()
{
    mpRenderer->SetViewportSize(width(), height());
}
