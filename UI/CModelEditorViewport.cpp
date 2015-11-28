#include "CModelEditorViewport.h"
#include <Core/CDrawUtil.h>

CModelEditorViewport::CModelEditorViewport(QWidget *pParent)
    : CBasicViewport(pParent),
      mMode(eDrawMesh),
      mpActiveMaterial(nullptr),
      mpModelNode(nullptr)
{
    mpRenderer = new CRenderer();
    mpRenderer->SetViewportSize(width(), height());
    mpRenderer->SetClearColor(CColor::skBlack);
    mpRenderer->ToggleGrid(true);

    mViewInfo.pRenderer = mpRenderer;
    mViewInfo.pScene = nullptr;
    mViewInfo.GameMode = false;
}

CModelEditorViewport::~CModelEditorViewport()
{
    delete mpRenderer;
}

void CModelEditorViewport::SetNode(CModelNode *pNode)
{
    mpModelNode = pNode;
}

void CModelEditorViewport::SetActiveMaterial(CMaterial *pMat)
{
    mpActiveMaterial = pMat;
}

void CModelEditorViewport::SetDrawMode(EDrawMode mode)
{
    mMode = mode;
}

void CModelEditorViewport::SetClearColor(CColor color)
{
    mpRenderer->SetClearColor(color);
}

void CModelEditorViewport::Paint()
{
    mpRenderer->BeginFrame();
    mCamera.LoadMatrices();

    if (!mpModelNode->Model())
        CDrawUtil::DrawGrid();

    else if (mMode == eDrawMesh)
    {
        CDrawUtil::DrawGrid();
        mpModelNode->AddToRenderer(mpRenderer, mViewInfo);
        mpRenderer->RenderBuckets(mViewInfo);
    }

    else if (mMode == eDrawSphere)
    {
        if (!mpActiveMaterial) return;
        glEnable(GL_CULL_FACE);

        CGraphics::sVertexBlock.COLOR0_Amb = CGraphics::skDefaultAmbientColor.ToVector4f();
        CGraphics::sMVPBlock.ModelMatrix = CMatrix4f::skIdentity;
        CGraphics::UpdateMVPBlock();
        CGraphics::SetDefaultLighting();
        CGraphics::UpdateLightBlock(); // Note: vertex block is updated by the material
        mpActiveMaterial->SetCurrent(eEnableUVScroll | eEnableBackfaceCull | eEnableOccluders);

        CDrawUtil::DrawSphere(true);
    }

    else if (mMode == eDrawSquare)
    {
        if (!mpActiveMaterial) return;
        glDisable(GL_CULL_FACE);

        CGraphics::SetDefaultLighting();
        CGraphics::UpdateLightBlock();
        CGraphics::sVertexBlock.COLOR0_Amb = CGraphics::skDefaultAmbientColor.ToVector4f();

        CGraphics::sMVPBlock.ModelMatrix = CMatrix4f::skIdentity;
        CGraphics::sMVPBlock.ViewMatrix = CMatrix4f::skIdentity;
        CGraphics::sMVPBlock.ProjectionMatrix = CMatrix4f::skIdentity;
        CGraphics::UpdateMVPBlock();

        mpActiveMaterial->SetCurrent(eEnableUVScroll | eEnableOccluders);
        float Aspect = (float) width() / (float) height();
        CDrawUtil::DrawSquare(CVector2f(0,1), CVector2f(1 * Aspect, 1), CVector2f(1 * Aspect, 0), CVector2f(0,0));
    }

    mpRenderer->EndFrame();
}

void CModelEditorViewport::OnResize()
{
    mpRenderer->SetViewportSize(width(), height());
}
