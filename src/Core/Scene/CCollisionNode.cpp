#include "CCollisionNode.h"
#include "CScene.h"
#include "Core/Render/CDrawUtil.h"
#include "Core/Render/CGraphics.h"
#include "Core/Render/CRenderer.h"

CCollisionNode::CCollisionNode(CScene *pScene, u32 NodeID, CSceneNode *pParent, CCollisionMeshGroup *pCollision)
    : CSceneNode(pScene, NodeID, pParent)
{
    SetCollision(pCollision);
    SetName("Collision");
}

ENodeType CCollisionNode::NodeType()
{
    return eCollisionNode;
}

void CCollisionNode::AddToRenderer(CRenderer *pRenderer, const SViewInfo& rkViewInfo)
{
    if (!mpCollision) return;
    if (!rkViewInfo.ViewFrustum.BoxInFrustum(AABox())) return;
    if (rkViewInfo.GameMode) return;

    pRenderer->AddMesh(this, -1, AABox(), false, eDrawMesh);

    if (mSelected)
        pRenderer->AddMesh(this, -1, AABox(), false, eDrawSelection);
}

void CCollisionNode::Draw(FRenderOptions /*Options*/, int /*ComponentIndex*/, ERenderCommand /*Command*/, const SViewInfo& rkViewInfo)
{
    if (!mpCollision) return;

    LoadModelMatrix();

    glBlendFunc(GL_ONE, GL_ZERO);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glDepthMask(GL_TRUE);

    // Turn off backface culling
    EGame Game = mpScene->ActiveArea()->Game();
    bool ForceDisableBackfaceCull = (rkViewInfo.CollisionSettings.DrawBackfaces || Game == EGame::DKCReturns) && glIsEnabled(GL_CULL_FACE);

    if (ForceDisableBackfaceCull)
        glDisable(GL_CULL_FACE);

    CColor BaseTint = TintColor(rkViewInfo);

    for (u32 iMesh = 0; iMesh < mpCollision->NumMeshes(); iMesh++)
    {
        CCollisionMesh *pMesh = mpCollision->MeshByIndex(iMesh);

        for (u32 iMat = 0; iMat < pMesh->NumMaterials(); iMat++)
        {
            CCollisionMaterial& rMat = pMesh->GetMaterial(iMat);

            if (rkViewInfo.CollisionSettings.HideMaterial & rMat)
                continue;

            if (rkViewInfo.CollisionSettings.HideMask != 0 && (rMat.RawFlags() & rkViewInfo.CollisionSettings.HideMask) != 0)
                continue;

            CColor Tint = BaseTint;

            if (rkViewInfo.CollisionSettings.HighlightMask != 0 && (rMat.RawFlags() & rkViewInfo.CollisionSettings.HighlightMask) == rkViewInfo.CollisionSettings.HighlightMask)
                Tint *= CColor::skRed;

            else if (Game != EGame::DKCReturns && rkViewInfo.CollisionSettings.TintWithSurfaceColor)
                Tint *= rMat.SurfaceColor(Game);

            bool IsFloor = (rkViewInfo.CollisionSettings.TintUnwalkableTris ? rMat.IsFloor() : true) || Game == EGame::DKCReturns;
            bool IsUnstandable = (rkViewInfo.CollisionSettings.TintUnwalkableTris ? rMat.IsUnstandable(Game) : false) && Game != EGame::DKCReturns;
            CDrawUtil::UseCollisionShader(IsFloor, IsUnstandable, Tint);
            pMesh->DrawMaterial(iMat, false);

            if (rkViewInfo.CollisionSettings.DrawWireframe)
                pMesh->DrawMaterial(iMat, true);
        }
    }

    // Restore backface culling setting
    if (ForceDisableBackfaceCull)
        glEnable(GL_CULL_FACE);

    // Draw collision bounds for area collision
    // note: right now checking parent is the best way to check whether this node is area collision instead of actor collision
    // actor collision will have a script node parent whereas area collision will have a root node parent
    if (rkViewInfo.CollisionSettings.DrawAreaCollisionBounds && Parent()->NodeType() == eRootNode && Game != EGame::DKCReturns)
        CDrawUtil::DrawWireCube( mpCollision->MeshByIndex(0)->BoundingBox(), CColor::skRed );
}

void CCollisionNode::RayAABoxIntersectTest(CRayCollisionTester& /*rTester*/, const SViewInfo& /*rkViewInfo*/)
{
    // todo
}

SRayIntersection CCollisionNode::RayNodeIntersectTest(const CRay& /*rkRay*/, u32 /*AssetID*/, const SViewInfo& /*rkViewInfo*/)
{
    // todo
    SRayIntersection Result;
    Result.Hit = false;
    return Result;
}

void CCollisionNode::SetCollision(CCollisionMeshGroup *pCollision)
{
    mpCollision = pCollision;
}
