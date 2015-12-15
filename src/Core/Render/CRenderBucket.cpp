#include "CRenderBucket.h"
#include "CDrawUtil.h"
#include "CGraphics.h"
#include "CRenderer.h"
#include <algorithm>

CRenderBucket::CRenderBucket()
{
    mEstSize = 0;
    mSize = 0;
}

void CRenderBucket::SetSortType(ESortType Type)
{
    mSortType = Type;
}

void CRenderBucket::Add(const SRenderablePtr& ptr)
{
    if (mSize >= mEstSize)
        mRenderables.push_back(ptr);
    else
        mRenderables[mSize] = ptr;

    mSize++;
}

void CRenderBucket::Sort(CCamera* pCamera)
{
    struct {
        CCamera *pCamera;
        bool operator()(SRenderablePtr left, SRenderablePtr right) {
            CVector3f cPos = pCamera->Position();
            CVector3f cDir = pCamera->Direction();

            CVector3f distL = left.AABox.ClosestPointAlongVector(cDir) - cPos;
            float dotL = distL.Dot(cDir);
            CVector3f distR = right.AABox.ClosestPointAlongVector(cDir) - cPos;
            float dotR = distR.Dot(cDir);
            return (dotL > dotR);
        }
    } backToFront;
    backToFront.pCamera = pCamera;

    if (mSortType == BackToFront)
        std::stable_sort(mRenderables.begin(), mRenderables.begin() + mSize, backToFront);

    // Test: draw node bounding boxes + vertices used for sorting
    /*for (u32 iNode = 0; iNode < mNodes.size(); iNode++)
    {
        SMeshPointer *pNode = &mNodes[iNode];
        CVector3f Vert = pNode->AABox.ClosestPointAlongVector(Camera.GetDirection());
        CDrawUtil::DrawWireCube(pNode->AABox, CColor::skWhite);

        CVector3f Dist = Vert - Camera.GetPosition();
        float Dot = Dist.Dot(Camera.GetDirection());
        if (Dot < 0.f) Dot = -Dot;
        if (Dot > 50.f) Dot = 50.f;
        float Intensity = 1.f - (Dot / 50.f);
        CColor CubeColor(Intensity, Intensity, Intensity, 1.f);

        CGraphics::sMVPBlock.ModelMatrix = CTransform4f::TranslationMatrix(Vert).ToMatrix4f();
        CGraphics::UpdateMVPBlock();
        CDrawUtil::DrawCube(CubeColor);
    }*/
}

void CRenderBucket::Clear()
{
    mEstSize = mSize;
    if (mRenderables.size() > mSize) mRenderables.resize(mSize);
    mSize = 0;
}

void CRenderBucket::Draw(const SViewInfo& ViewInfo)
{
    ERenderOptions Options = ViewInfo.pRenderer->RenderOptions();

    for (u32 n = 0; n < mSize; n++)
    {
        if (mRenderables[n].Command == eDrawMesh)
            mRenderables[n].pRenderable->Draw(Options, mRenderables[n].ComponentIndex, ViewInfo);

        else if (mRenderables[n].Command == eDrawSelection)
            mRenderables[n].pRenderable->DrawSelection();

        // todo: implementation for eDrawExtras
    }
}
