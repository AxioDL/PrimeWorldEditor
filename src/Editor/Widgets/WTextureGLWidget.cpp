#include "WTextureGLWidget.h"
#include <Common/Math/CTransform4f.h>
#include <Core/GameProject/CResourceStore.h>
#include <Core/Render/CDrawUtil.h>
#include <Core/Render/CGraphics.h>
#include <iostream>
#include <iomanip>

WTextureGLWidget::WTextureGLWidget(QWidget *pParent, CTexture *pTex)
    : QOpenGLWidget(pParent)
{
    SetTexture(pTex);
    mInitialized = false;
}

WTextureGLWidget::~WTextureGLWidget()
{
    if (mInitialized) CGraphics::ReleaseContext(mContextID);
}

void WTextureGLWidget::initializeGL()
{
    CGraphics::Initialize();
    glEnable(GL_BLEND);
    mContextID = CGraphics::GetContextIndex();
    mInitialized = true;
}

void WTextureGLWidget::paintGL()
{
    CGraphics::SetActiveContext(mContextID);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(1.f, 0.f, 0.f, 0.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);

    // Set matrices to identity
    CGraphics::sMVPBlock.ModelMatrix = CMatrix4f::skIdentity;
    CGraphics::sMVPBlock.ViewMatrix = CMatrix4f::skIdentity;
    CGraphics::sMVPBlock.ProjectionMatrix = CMatrix4f::skIdentity;
    CGraphics::UpdateMVPBlock();

    // Draw checkerboard background
    CDrawUtil::UseTextureShader();
    glDepthMask(GL_FALSE);
    CDrawUtil::LoadCheckerboardTexture(0);
    CDrawUtil::DrawSquare(&mCheckerCoords[0].X);

    // Make it darker
    CDrawUtil::UseColorShader(CColor::Integral(0.0f, 0.0f, 0.0f, 0.5f));
    glDepthMask(GL_FALSE);
    CDrawUtil::DrawSquare();

    // Leave it at just the checkerboard if there's no texture
    if (!mpTexture) return;

    // Draw texture
    CDrawUtil::UseTextureShader();
    mpTexture->Bind(0);
    CGraphics::sMVPBlock.ModelMatrix = mTexTransform;
    CGraphics::UpdateMVPBlock();
    CDrawUtil::DrawSquare();

    glEnable(GL_DEPTH_TEST);
}

void WTextureGLWidget::resizeGL(int Width, int Height)
{
    mAspectRatio = (float) Width / (float) Height;
    glViewport(0, 0, Width, Height);

    CalcTexTransform();
    CalcCheckerCoords();
    update();
}

void WTextureGLWidget::SetTexture(CTexture *pTex)
{
    mpTexture = pTex;

    if (pTex) mTexAspectRatio = (float) pTex->Width() / (float) pTex->Height();
    else mTexAspectRatio = 0.f;

    CalcTexTransform();
    CalcCheckerCoords();
    update();
}

void WTextureGLWidget::CalcTexTransform()
{
    // This is a simple scale based on the dimensions of the viewport, in order to
    // avoid stretching the texture if it doesn't match the viewport aspect ratio.
    mTexTransform = CTransform4f::skIdentity;
    float Diff = mTexAspectRatio / mAspectRatio;

    if (mAspectRatio >= mTexAspectRatio)
        mTexTransform.Scale(Diff, 1.f, 1.f);
    else
        mTexTransform.Scale(1.f, 1.f / Diff, 1.f);
}

void WTextureGLWidget::CalcCheckerCoords()
{
    // The translation vector is set up so the checkerboard stays centered on the screen
    // rather than expanding from the bottom-left corner. This makes it look more natural.
    CVector2f Trans;
    float InvAspect    = (mAspectRatio == 0.f) ? 0.f : 1.f / mAspectRatio;
    float InvTexAspect = (mTexAspectRatio == 0.f) ? 0.f : 1.f / mTexAspectRatio;
    float XBase, YBase, XScale, YScale;

    // Horizontal texture
    if ((mpTexture != nullptr) && (mpTexture->Width() > mpTexture->Height()))
    {
        XBase = 1.f;
        YBase = InvTexAspect;
        XScale = InvTexAspect;
        YScale = 1.f;
    }
    // Vertical texture
    else
    {
        XBase = mTexAspectRatio;
        YBase = 1.f;
        XScale = 1.f;
        YScale = mTexAspectRatio;
    }

    // Space on left/right
    if (mAspectRatio > mTexAspectRatio)
    {
        Trans = CVector2f(mAspectRatio / 2.f, 0.5f) * -XScale;
        mCheckerCoords[0] = CVector2f(0.f, YBase);
        mCheckerCoords[1] = CVector2f(mAspectRatio * XScale, YBase);
        mCheckerCoords[2] = CVector2f(mAspectRatio * XScale, 0.f);
        mCheckerCoords[3] = CVector2f(0.f, 0.f);
    }

    // Space on top/bottom
    else
    {
        Trans = CVector2f(0.5f, InvAspect / 2.f) * -YScale;
        mCheckerCoords[0] = CVector2f(0.f, InvAspect * YScale);
        mCheckerCoords[1] = CVector2f(XBase, InvAspect * YScale);
        mCheckerCoords[2] = CVector2f(XBase, 0.f);
        mCheckerCoords[3] = CVector2f(0.f, 0.f);
    }

    // Finally, apply translation/scale
    for (uint32 iCoord = 0; iCoord < 4; iCoord++)
    {
        mCheckerCoords[iCoord] += Trans;
        mCheckerCoords[iCoord] *= 10.f;
    }

}
