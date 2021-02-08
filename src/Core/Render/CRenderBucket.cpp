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
                     [&, pkCamera](const auto& rkLeft, const auto& rkRight) {
                         const CVector3f CamPos = pkCamera->Position();
                         const CVector3f CamDir = pkCamera->Direction();

                         const CVector3f DistL = rkLeft.AABox.ClosestPointAlongVector(CamDir) - CamPos;
                         const CVector3f DistR = rkRight.AABox.ClosestPointAlongVector(CamDir) - CamPos;
                         const float DotL = DistL.Dot(CamDir);
                         const float DotR = DistR.Dot(CamDir);
                         return DotL > DotR;
                     });

    if (!DebugVisualization)
        return;

    for (size_t iPtr = 0; iPtr < mSize; iPtr++)
    {
        const SRenderablePtr *pPtr = &mRenderables[iPtr];
        const CVector3f Point = pPtr->AABox.ClosestPointAlongVector(pkCamera->Direction());
        CDrawUtil::DrawWireCube(pPtr->AABox, CColor::White());

        const CVector3f Dist = Point - pkCamera->Position();
        float Dot = Dist.Dot(pkCamera->Direction());
        if (Dot < 0.f)
            Dot = -Dot;
        Dot = std::min(Dot, 50.f);
        const float Intensity = 1.f - (Dot / 50.f);
        const CColor CubeColor(Intensity, Intensity, Intensity, 1.f);

        CGraphics::sMVPBlock.ModelMatrix = CTransform4f::TranslationMatrix(Point);
        CGraphics::UpdateMVPBlock();
        CDrawUtil::DrawCube(CubeColor);
    }
}

void CRenderBucket::CSubBucket::Clear()
{
    mEstSize = mSize;

    if (mRenderables.size() > mSize)
        mRenderables.resize(mSize);

    mSize = 0;
}

void CRenderBucket::CSubBucket::Draw(const SViewInfo& rkViewInfo)
{
    const FRenderOptions Options = rkViewInfo.pRenderer->RenderOptions();

    for (size_t iPtr = 0; iPtr < mSize; iPtr++)
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
