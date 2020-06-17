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
    mScale = CVector3f::One();
    SetName("Static Node");
}

ENodeType CStaticNode::NodeType()
{
    return ENodeType::Static;
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
    if (!mpModel)
        return;

    if (mpModel->IsOccluder())
        return;

    if (!rkViewInfo.ViewFrustum.BoxInFrustum(AABox()))
        return;

    if (!mpModel->IsTransparent())
    {
        pRenderer->AddMesh(this, -1, AABox(), false, ERenderCommand::DrawMesh);
    }
    else
    {
        const size_t NumSurfaces = mpModel->GetSurfaceCount();
        for (uint32 iSurf = 0; iSurf < NumSurfaces; iSurf++)
        {
            CAABox TransformedBox = mpModel->GetSurfaceAABox(iSurf).Transformed(Transform());

            if (rkViewInfo.ViewFrustum.BoxInFrustum(TransformedBox))
                pRenderer->AddMesh(this, iSurf, TransformedBox, true, ERenderCommand::DrawMesh);
        }
    }

    if (mSelected && !rkViewInfo.GameMode)
        pRenderer->AddMesh(this, -1, AABox(), false, ERenderCommand::DrawSelection);
}

void CStaticNode::Draw(FRenderOptions Options, int ComponentIndex, ERenderCommand /*Command*/, const SViewInfo& rkViewInfo)
{
    if (!mpModel) return;

    bool IsLightingEnabled = CGraphics::sLightMode == CGraphics::ELightingMode::World || rkViewInfo.GameMode;
    bool UseWhiteAmbient   = (mpModel->GetMaterial()->Options() & EMaterialOption::DrawWhiteAmbientDKCR) != 0;

    if (IsLightingEnabled)
    {
        CGraphics::sNumLights = 0;
        CGraphics::sVertexBlock.COLOR0_Amb = UseWhiteAmbient ? CColor::TransparentWhite() : CColor::TransparentBlack();
        CGraphics::sVertexBlock.COLOR0_Mat = CColor::Black();
        CGraphics::sPixelBlock.LightmapMultiplier = 1.0f;
        CGraphics::UpdateLightBlock();
    }

    else
    {
        LoadLights(rkViewInfo);
        if (CGraphics::sLightMode == CGraphics::ELightingMode::None || UseWhiteAmbient)
            CGraphics::sVertexBlock.COLOR0_Amb = CColor::TransparentWhite();
        CGraphics::sVertexBlock.COLOR0_Mat = CColor::White();
    }

    float Mul = CGraphics::sWorldLightMultiplier;
    CGraphics::sPixelBlock.SetAllTevColors(CColor(Mul,Mul,Mul));
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
    mpModel->DrawWireframe(ERenderOption::None, WireframeColor());
}

void CStaticNode::RayAABoxIntersectTest(CRayCollisionTester& rTester, const SViewInfo& /*rkViewInfo*/)
{
    if (!mpModel || mpModel->IsOccluder())
        return;

    const CRay& rkRay = rTester.Ray();
    const std::pair<bool, float> BoxResult = AABox().IntersectsRay(rkRay);

    if (!BoxResult.first)
        return;

    for (uint32 iSurf = 0; iSurf < mpModel->GetSurfaceCount(); iSurf++)
    {
        const auto [intersects, distance] = mpModel->GetSurfaceAABox(iSurf).Transformed(Transform()).IntersectsRay(rkRay);

        if (intersects)
            rTester.AddNode(this, iSurf, distance);
    }
}

SRayIntersection CStaticNode::RayNodeIntersectTest(const CRay& rkRay, uint32 AssetID, const SViewInfo& rkViewInfo)
{
    SRayIntersection Out;
    Out.pNode = this;
    Out.ComponentIndex = AssetID;

    const CRay TransformedRay = rkRay.Transformed(Transform().Inverse());
    const FRenderOptions Options = rkViewInfo.pRenderer->RenderOptions();
    const auto [intersects, distance] = mpModel->GetSurface(AssetID)->IntersectsRay(TransformedRay, ((Options & ERenderOption::EnableBackfaceCull) == 0));

    if (intersects)
    {
        Out.Hit = true;

        const CVector3f HitPoint = TransformedRay.PointOnRay(distance);
        const CVector3f WorldHitPoint = Transform() * HitPoint;
        Out.Distance = Math::Distance(rkRay.Origin(), WorldHitPoint);
    }
    else
    {
        Out.Hit = false;
    }

    return Out;
}
