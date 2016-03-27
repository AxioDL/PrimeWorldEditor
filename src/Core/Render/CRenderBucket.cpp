#include "CRenderBucket.h"
#include "CDrawUtil.h"
#include "CGraphics.h"
#include "CRenderer.h"
#include <algorithm>

void CRenderBucket::Add(const SRenderablePtr& rkPtr)
{
    if (mSize >= mEstSize)
        mRenderables.push_back(rkPtr);
    else
        mRenderables[mSize] = rkPtr;

    mSize++;
}

void CRenderBucket::Sort(CCamera* pCamera)
{
    if (mEnableDepthSort)
    {
        std::stable_sort(mRenderables.begin(), mRenderables.begin() + mSize,
                         [&, pCamera](const SRenderablePtr& rkLeft, const SRenderablePtr& rkRight) -> bool
        {
            CVector3f CamPos = pCamera->Position();
            CVector3f CamDir = pCamera->Direction();

            CVector3f DistL = rkLeft.AABox.ClosestPointAlongVector(CamDir) - CamPos;
            CVector3f DistR = rkRight.AABox.ClosestPointAlongVector(CamDir) - CamPos;
            float DotL = DistL.Dot(CamDir);
            float DotR = DistR.Dot(CamDir);
            return (DotL > DotR);
        });

        if (mEnableDepthSortDebugVisualization)
        {
            for (u32 iPtr = 0; iPtr < mSize; iPtr++)
            {
                SRenderablePtr *pPtr = &mRenderables[iPtr];
                CVector3f Point = pPtr->AABox.ClosestPointAlongVector(pCamera->Direction());
                CDrawUtil::DrawWireCube(pPtr->AABox, CColor::skWhite);

                CVector3f Dist = Point - pCamera->Position();
                float Dot = Dist.Dot(pCamera->Direction());
                if (Dot < 0.f) Dot = -Dot;
                if (Dot > 50.f) Dot = 50.f;
                float Intensity = 1.f - (Dot / 50.f);
                CColor CubeColor(Intensity, Intensity, Intensity, 1.f);

                CGraphics::sMVPBlock.ModelMatrix = CTransform4f::TranslationMatrix(Point).ToMatrix4f();
                CGraphics::UpdateMVPBlock();
                CDrawUtil::DrawCube(CubeColor);
            }
        }
    }
}

void CRenderBucket::Clear()
{
    mEstSize = mSize;
    if (mRenderables.size() > mSize) mRenderables.resize(mSize);
    mSize = 0;
}

void CRenderBucket::Draw(const SViewInfo& rkViewInfo)
{
    FRenderOptions Options = rkViewInfo.pRenderer->RenderOptions();

    for (u32 iPtr = 0; iPtr < mSize; iPtr++)
    {
        if (mRenderables[iPtr].Command == eDrawMesh)
            mRenderables[iPtr].pRenderable->Draw(Options, mRenderables[iPtr].ComponentIndex, rkViewInfo);

        else if (mRenderables[iPtr].Command == eDrawSelection)
            mRenderables[iPtr].pRenderable->DrawSelection();
    }
}
