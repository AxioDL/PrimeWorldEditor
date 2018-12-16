#include "CRenderBucket.h"
#include "CDrawUtil.h"
#include "CGraphics.h"
#include "CRenderer.h"
#include <algorithm>

// ************ CSubBucket ************
void CRenderBucket::CSubBucket::Add(const SRenderablePtr& rkPtr)
{
    if (mSize >= mEstSize)
        mRenderables.push_back(rkPtr);
    else
        mRenderables[mSize] = rkPtr;

    mSize++;
}

void CRenderBucket::CSubBucket::Sort(const CCamera* pkCamera, bool DebugVisualization)
{
    std::stable_sort(mRenderables.begin(), mRenderables.begin() + mSize,
                     [&, pkCamera](const SRenderablePtr& rkLeft, const SRenderablePtr& rkRight) -> bool
    {
        CVector3f CamPos = pkCamera->Position();
        CVector3f CamDir = pkCamera->Direction();

        CVector3f DistL = rkLeft.AABox.ClosestPointAlongVector(CamDir) - CamPos;
        CVector3f DistR = rkRight.AABox.ClosestPointAlongVector(CamDir) - CamPos;
        float DotL = DistL.Dot(CamDir);
        float DotR = DistR.Dot(CamDir);
        return (DotL > DotR);
    });

    if (DebugVisualization)
    {
        for (uint32 iPtr = 0; iPtr < mSize; iPtr++)
        {
            SRenderablePtr *pPtr = &mRenderables[iPtr];
            CVector3f Point = pPtr->AABox.ClosestPointAlongVector(pkCamera->Direction());
            CDrawUtil::DrawWireCube(pPtr->AABox, CColor::skWhite);

            CVector3f Dist = Point - pkCamera->Position();
            float Dot = Dist.Dot(pkCamera->Direction());
            if (Dot < 0.f) Dot = -Dot;
            if (Dot > 50.f) Dot = 50.f;
            float Intensity = 1.f - (Dot / 50.f);
            CColor CubeColor(Intensity, Intensity, Intensity, 1.f);

            CGraphics::sMVPBlock.ModelMatrix = CTransform4f::TranslationMatrix(Point);
            CGraphics::UpdateMVPBlock();
            CDrawUtil::DrawCube(CubeColor);
        }
    }
}

void CRenderBucket::CSubBucket::Clear()
{
    mEstSize = mSize;
    if (mRenderables.size() > mSize) mRenderables.resize(mSize);
    mSize = 0;
}

void CRenderBucket::CSubBucket::Draw(const SViewInfo& rkViewInfo)
{
    FRenderOptions Options = rkViewInfo.pRenderer->RenderOptions();

    for (uint32 iPtr = 0; iPtr < mSize; iPtr++)
    {
        const SRenderablePtr& rkPtr = mRenderables[iPtr];

        // todo: DrawSelection probably shouldn't be a separate function anymore.
        if (rkPtr.Command == ERenderCommand::DrawSelection)
            rkPtr.pRenderable->DrawSelection();
        else
            rkPtr.pRenderable->Draw(Options, rkPtr.ComponentIndex, rkPtr.Command, rkViewInfo);
    }
}

// ************ CRenderBucket ************
void CRenderBucket::Add(const SRenderablePtr& rkPtr, bool Transparent)
{
    if (Transparent)
        mTransparentSubBucket.Add(rkPtr);
    else
        mOpaqueSubBucket.Add(rkPtr);
}

void CRenderBucket::Clear()
{
    mOpaqueSubBucket.Clear();
    mTransparentSubBucket.Clear();
}

void CRenderBucket::Draw(const SViewInfo& rkViewInfo)
{
    mOpaqueSubBucket.Draw(rkViewInfo);
    mTransparentSubBucket.Sort(rkViewInfo.pCamera, mEnableDepthSortDebugVisualization);
    mTransparentSubBucket.Draw(rkViewInfo);
}
