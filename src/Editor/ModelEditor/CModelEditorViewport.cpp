#include "CModelEditorViewport.h"
#include <Core/Render/CDrawUtil.h>

CModelEditorViewport::CModelEditorViewport(QWidget *pParent)
    : CBasicViewport(pParent), mpRenderer{std::make_unique<CRenderer>()}
{
    const qreal pixelRatio = devicePixelRatioF();
    mpRenderer->SetViewportSize(width() * pixelRatio, height() * pixelRatio);
    mpRenderer->SetClearColor(CColor::Black());
    mpRenderer->ToggleGrid(true);

    mViewInfo.pRenderer = mpRenderer.get();
    mViewInfo.pScene = nullptr;
    mViewInfo.GameMode = false;
}

CModelEditorViewport::~CModelEditorViewport() = default;

void CModelEditorViewport::SetNode(CModelNode *pNode)
{
    mpModelNode = pNode;
}

void CModelEditorViewport::SetActiveMaterial(CMaterial *pMat)
{
    mpActiveMaterial = pMat;
}

void CModelEditorViewport::SetDrawMode(EDrawMode Mode)
{
    mMode = Mode;
}

void CModelEditorViewport::SetClearColor(CColor Color)
{
    mpRenderer->SetClearColor(Color);
}

void CModelEditorViewport::SetGridEnabled(bool Enable)
{
    mGridEnabled = Enable;
}

void CModelEditorViewport::Paint()
{
    mpRenderer->BeginFrame();
    mCamera.LoadMatrices();

    if (!mpModelNode->Model())
    {
        if (mGridEnabled)
            mGrid.AddToRenderer(mpRenderer.get(), mViewInfo);

        mpRenderer->RenderBuckets(mViewInfo);
    }
    else if (mMode == EDrawMode::DrawMesh)
    {
        if (mGridEnabled)
            mGrid.AddToRenderer(mpRenderer.get(), mViewInfo);

        mpModelNode->AddToRenderer(mpRenderer.get(), mViewInfo);
        mpRenderer->RenderBuckets(mViewInfo);
    }
    else if (mMode == EDrawMode::DrawSphere)
    {
        if (!mpActiveMaterial) return;
        glEnable(GL_CULL_FACE);

        CGraphics::sVertexBlock.COLOR0_Amb = CGraphics::skDefaultAmbientColor;
        CGraphics::sMVPBlock.ModelMatrix = CMatrix4f::skIdentity;
        CGraphics::UpdateMVPBlock();
        CGraphics::SetDefaultLighting();
        CGraphics::UpdateLightBlock(); // Note: vertex block is updated by the material

        for (CMaterial* passMat = mpActiveMaterial; passMat; passMat = passMat->GetNextDrawPass())
        {
            passMat->SetCurrent(ERenderOption::EnableUVScroll | ERenderOption::EnableBackfaceCull | ERenderOption::EnableOccluders);
            CDrawUtil::DrawSphere(true);
        }
    }
    else if (mMode == EDrawMode::DrawSquare)
    {
        if (!mpActiveMaterial) return;
        glDisable(GL_CULL_FACE);

        CGraphics::SetDefaultLighting();
        CGraphics::UpdateLightBlock();
        CGraphics::sVertexBlock.COLOR0_Amb = CGraphics::skDefaultAmbientColor;

        CGraphics::sMVPBlock.ModelMatrix = CMatrix4f::skIdentity;
        CGraphics::sMVPBlock.ViewMatrix = CMatrix4f::skIdentity;
        CGraphics::sMVPBlock.ProjectionMatrix = CMatrix4f::skIdentity;
        CGraphics::UpdateMVPBlock();

        for (CMaterial* passMat = mpActiveMaterial; passMat; passMat = passMat->GetNextDrawPass())
        {
            passMat->SetCurrent(ERenderOption::EnableUVScroll | ERenderOption::EnableOccluders);
            float Aspect = (float) width() / (float) height();
            CDrawUtil::DrawSquare(CVector2f(0,1), CVector2f(1 * Aspect, 1), CVector2f(1 * Aspect, 0), CVector2f(0,0));
        }
    }

    mpRenderer->EndFrame();
}

void CModelEditorViewport::OnResize()
{
    qreal pixelRatio = devicePixelRatioF();
    mpRenderer->SetViewportSize(width() * pixelRatio, height() * pixelRatio);
}
