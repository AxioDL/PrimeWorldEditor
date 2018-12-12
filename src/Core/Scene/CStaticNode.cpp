#include "CStaticNode.h"
#include "Core/Render/CGraphics.h"
#include "Core/Render/CDrawUtil.h"
#include "Core/Render/CRenderer.h"
#include <Common/Math/MathUtil.h>

CStaticNode::CStaticNode(CScene *pScene, uint32 NodeID, CSceneNode *pParent, CStaticModel *pModel)
    : CSceneNode(pScene, NodeID, pParent)
    , mpModel(pModel)
{
    mLocalAABox = mpModel->AABox();
    mScale = CVector3f::skOne;
    SetName("Static Node");
}

ENodeType CStaticNode::NodeType()
{
    return eStaticNode;
}

void CStaticNode::PostLoad()
{
    if (mpModel)
    {
        mpModel->BufferGL();
        mpModel->GenerateMaterialShaders();
    }
}

void CStaticNode::AddToRenderer(CRenderer *pRenderer, const SViewInfo& rkViewInfo)
{
    if (!mpModel) return;
    if (mpModel->IsOccluder()) return;
    if (!rkViewInfo.ViewFrustum.BoxInFrustum(AABox())) return;

    if (!mpModel->IsTransparent())
        pRenderer->AddMesh(this, -1, AABox(), false, eDrawMesh);

    else
    {
        uint32 NumSurfaces = mpModel->GetSurfaceCount();
        for (uint32 iSurf = 0; iSurf < NumSurfaces; iSurf++)
        {
            CAABox TransformedBox = mpModel->GetSurfaceAABox(iSurf).Transformed(Transform());

            if (rkViewInfo.ViewFrustum.BoxInFrustum(TransformedBox))
                pRenderer->AddMesh(this, iSurf, TransformedBox, true, eDrawMesh);
        }
    }

    if (mSelected && !rkViewInfo.GameMode)
        pRenderer->AddMesh(this, -1, AABox(), false, eDrawSelection);
}

void CStaticNode::Draw(FRenderOptions Options, int ComponentIndex, ERenderCommand /*Command*/, const SViewInfo& rkViewInfo)
{
    if (!mpModel) return;

    bool IsLightingEnabled = CGraphics::sLightMode == CGraphics::eWorldLighting || rkViewInfo.GameMode;
    bool UseWhiteAmbient   = (mpModel->GetMaterial()->Options() & CMaterial::eDrawWhiteAmbientDKCR) != 0;

    if (IsLightingEnabled)
    {
        CGraphics::sNumLights = 0;
        CGraphics::sVertexBlock.COLOR0_Amb = UseWhiteAmbient ? CColor::skWhite : CColor::skBlack;
        CGraphics::sPixelBlock.LightmapMultiplier = 1.0f;
        CGraphics::UpdateLightBlock();
    }

    else
    {
        LoadLights(rkViewInfo);
        if (CGraphics::sLightMode == CGraphics::eNoLighting || UseWhiteAmbient)
            CGraphics::sVertexBlock.COLOR0_Amb = CColor::skWhite;
    }

    float Mul = CGraphics::sWorldLightMultiplier;
    CGraphics::sPixelBlock.TevColor = CColor(Mul,Mul,Mul);
    CGraphics::sPixelBlock.TintColor = TintColor(rkViewInfo);
    LoadModelMatrix();

    if (ComponentIndex < 0)
        mpModel->Draw(Options);
    else
        mpModel->DrawSurface(Options, ComponentIndex);
}

void CStaticNode::DrawSelection()
{
    if (!mpModel) return;
    LoadModelMatrix();
    mpModel->DrawWireframe(eNoRenderOptions, WireframeColor());
}

void CStaticNode::RayAABoxIntersectTest(CRayCollisionTester& rTester, const SViewInfo& /*rkViewInfo*/)
{
    if ((!mpModel) || (mpModel->IsOccluder()))
        return;

    const CRay& rkRay = rTester.Ray();
    std::pair<bool,float> BoxResult = AABox().IntersectsRay(rkRay);

    if (BoxResult.first)
    {
        for (uint32 iSurf = 0; iSurf < mpModel->GetSurfaceCount(); iSurf++)
        {
            std::pair<bool,float> SurfResult = mpModel->GetSurfaceAABox(iSurf).Transformed(Transform()).IntersectsRay(rkRay);

            if (SurfResult.first)
                rTester.AddNode(this, iSurf, SurfResult.second);
        }
    }
}

SRayIntersection CStaticNode::RayNodeIntersectTest(const CRay& rkRay, uint32 AssetID, const SViewInfo& rkViewInfo)
{
    SRayIntersection Out;
    Out.pNode = this;
    Out.ComponentIndex = AssetID;

    CRay TransformedRay = rkRay.Transformed(Transform().Inverse());
    FRenderOptions Options = rkViewInfo.pRenderer->RenderOptions();
    std::pair<bool,float> Result = mpModel->GetSurface(AssetID)->IntersectsRay(TransformedRay, ((Options & eEnableBackfaceCull) == 0));

    if (Result.first)
    {
        Out.Hit = true;

        CVector3f HitPoint = TransformedRay.PointOnRay(Result.second);
        CVector3f WorldHitPoint = Transform() * HitPoint;
        Out.Distance = Math::Distance(rkRay.Origin(), WorldHitPoint);
    }

    else
        Out.Hit = false;

    return Out;
}
