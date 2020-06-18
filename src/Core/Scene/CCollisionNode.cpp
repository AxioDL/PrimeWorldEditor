#include "CCollisionNode.h"
#include "CScene.h"
#include "Core/Render/CDrawUtil.h"
#include "Core/Render/CGraphics.h"
#include "Core/Render/CRenderer.h"

CCollisionNode::CCollisionNode(CScene *pScene, uint32 NodeID, CSceneNode *pParent, CCollisionMeshGroup *pCollision)
    : CSceneNode(pScene, NodeID, pParent)
{
    SetCollision(pCollision);
    SetName("Collision");
}

ENodeType CCollisionNode::NodeType()
{
    return ENodeType::Collision;
}

void CCollisionNode::AddToRenderer(CRenderer *pRenderer, const SViewInfo& rkViewInfo)
{
    if (!mpCollision) return;
    if (!rkViewInfo.ViewFrustum.BoxInFrustum(AABox())) return;
    if (rkViewInfo.GameMode) return;

    pRenderer->AddMesh(this, -1, AABox(), false, ERenderCommand::DrawMesh);

    if (mSelected)
        pRenderer->AddMesh(this, -1, AABox(), false, ERenderCommand::DrawSelection);
}

void CCollisionNode::Draw(FRenderOptions /*Options*/, int /*ComponentIndex*/, ERenderCommand /*Command*/, const SViewInfo& rkViewInfo)
{
    if (!mpCollision) return;

    LoadModelMatrix();

    glBlendFunc(GL_ONE, GL_ZERO);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glDepthMask(GL_TRUE);

    // Turn off backface culling
    EGame Game = mpCollision->Game();
    bool ForceDisableBackfaceCull = (rkViewInfo.CollisionSettings.DrawBackfaces || Game == EGame::DKCReturns) && glIsEnabled(GL_CULL_FACE);

    if (ForceDisableBackfaceCull)
        glDisable(GL_CULL_FACE);

    CColor BaseTint = TintColor(rkViewInfo);

    for (size_t MeshIdx = 0; MeshIdx < mpCollision->NumMeshes(); MeshIdx++)
    {
        CCollisionMesh *pMesh = mpCollision->MeshByIndex(MeshIdx);
        CCollisionRenderData& RenderData = pMesh->GetRenderData();
        const SCollisionIndexData& kIndexData = pMesh->GetIndexData();

        for (int MatIdx = 0; MatIdx < static_cast<int>(kIndexData.Materials.size()); MatIdx++)
        {
            const CCollisionMaterial& kMat = kIndexData.Materials[MatIdx];

            if (rkViewInfo.CollisionSettings.HideMaterial & kMat)
                continue;

            if (rkViewInfo.CollisionSettings.HideMask != 0 && (kMat.RawFlags() & rkViewInfo.CollisionSettings.HideMask) != 0)
                continue;

            CColor Tint = BaseTint;

            if (rkViewInfo.CollisionSettings.HighlightMask != 0 && (kMat.RawFlags() & rkViewInfo.CollisionSettings.HighlightMask) == rkViewInfo.CollisionSettings.HighlightMask)
                Tint *= CColor::Red();
            else if (Game != EGame::DKCReturns && rkViewInfo.CollisionSettings.TintWithSurfaceColor)
                Tint *= kMat.SurfaceColor(Game);

            bool IsFloor = (rkViewInfo.CollisionSettings.TintUnwalkableTris ? kMat.IsFloor() : true) || Game == EGame::DKCReturns;
            bool IsUnstandable = (rkViewInfo.CollisionSettings.TintUnwalkableTris ? kMat.IsUnstandable(Game) : false) && Game != EGame::DKCReturns;
            CDrawUtil::UseCollisionShader(IsFloor, IsUnstandable, Tint);
            RenderData.Render(false, MatIdx);

            if (rkViewInfo.CollisionSettings.DrawWireframe)
                RenderData.Render(true, MatIdx);
        }
    }

    // Render bounding hierarchy
    if (rkViewInfo.CollisionSettings.DrawBoundingHierarchy)
    {
        const int Depth = rkViewInfo.CollisionSettings.BoundingHierarchyRenderDepth;

        for (size_t MeshIdx = 0; MeshIdx < mpCollision->NumMeshes(); MeshIdx++)
        {
            mpCollision->MeshByIndex(MeshIdx)->GetRenderData().RenderBoundingHierarchy(Depth);
        }
    }

    // Restore backface culling setting
    if (ForceDisableBackfaceCull)
        glEnable(GL_CULL_FACE);

    // Draw collision bounds for area collision
    // note: right now checking parent is the best way to check whether this node is area collision instead of actor collision
    // actor collision will have a script node parent whereas area collision will have a root node parent
    if (rkViewInfo.CollisionSettings.DrawAreaCollisionBounds)
    {
        if (Parent() && Parent()->NodeType() == ENodeType::Root && Game != EGame::DKCReturns)
        {
            CDrawUtil::DrawWireCube(mpCollision->MeshByIndex(0)->Bounds(), CColor::Red());
        }
    }
}

void CCollisionNode::RayAABoxIntersectTest(CRayCollisionTester& /*rTester*/, const SViewInfo& /*rkViewInfo*/)
{
    // todo
}

SRayIntersection CCollisionNode::RayNodeIntersectTest(const CRay& /*rkRay*/, uint32 /*AssetID*/, const SViewInfo& /*rkViewInfo*/)
{
    // todo
    SRayIntersection Result;
    Result.Hit = false;
    return Result;
}

void CCollisionNode::SetCollision(CCollisionMeshGroup *pCollision)
{
    mpCollision = pCollision;

    if (!mpCollision)
        return;

    mpCollision->BuildRenderData();

    // Update bounds
    mLocalAABox = CAABox::Infinite();

    for (size_t MeshIdx = 0; MeshIdx < pCollision->NumMeshes(); MeshIdx++)
    {
        const CCollisionMesh* pMesh = pCollision->MeshByIndex(MeshIdx);
        mLocalAABox.ExpandBounds(pMesh->Bounds());
    }
}
