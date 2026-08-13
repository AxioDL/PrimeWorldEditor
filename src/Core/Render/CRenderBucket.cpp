#include "Core/Render/CRenderBucket.h"

#include "Core/Render/CCamera.h"
#include "Core/Render/CDrawUtil.h"
#include "Core/Render/CGraphics.h"
#include "Core/Render/CRenderer.h"
#include "Core/Render/IRenderable.h"
#include "Core/Render/SRenderablePtr.h"
#include "Core/Render/SViewInfo.h"

#include <algorithm>

// ************ CSubBucket ************
CRenderBucket::CSubBucket::CSubBucket() = default;
CRenderBucket::CSubBucket::~CSubBucket() = default;

void CRenderBucket::CSubBucket::Add(const SRenderablePtr& rkPtr)
{
    mRenderables.push_back(rkPtr);
}

void CRenderBucket::CSubBucket::Sort(const CCamera* pkCamera, bool DebugVisualization)
{
    std::ranges::stable_sort(mRenderables, [pkCamera](const auto& rkLeft, const auto& rkRight) {
        const CVector3f& CamPos = pkCamera->Position();
        const CVector3f& CamDir = pkCamera->Direction();

        const CVector3f DistL = rkLeft.AABox.ClosestPointAlongVector(CamDir) - CamPos;
        const CVector3f DistR = rkRight.AABox.ClosestPointAlongVector(CamDir) - CamPos;
        const float DotL = DistL.Dot(CamDir);
        const float DotR = DistR.Dot(CamDir);
        return DotL > DotR;
    });

    if (!DebugVisualization)
        return;

    for (const auto& ptr : mRenderables)
    {
        const CVector3f Point = ptr.AABox.ClosestPointAlongVector(pkCamera->Direction());
        CDrawUtil::DrawWireCube(ptr.AABox, CColor::White());

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
    mRenderables.clear();
}

void CRenderBucket::CSubBucket::Draw(const SViewInfo& rkViewInfo)
{
    const FRenderOptions Options = rkViewInfo.pRenderer->RenderOptions();

    for (const auto& ptr : mRenderables)
        ptr.pRenderable->Draw(Options, ptr.ComponentIndex, ptr.Command, rkViewInfo);
}

// ************ CRenderBucket ************
CRenderBucket::CRenderBucket() = default;
CRenderBucket::~CRenderBucket() = default;

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
