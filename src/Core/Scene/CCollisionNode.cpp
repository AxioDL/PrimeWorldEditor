#include "CCollisionNode.h"
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

void CCollisionNode::AddToRenderer(CRenderer *pRenderer, const SViewInfo& ViewInfo)
{
    if (!mpCollision) return;
    if (!ViewInfo.ViewFrustum.BoxInFrustum(AABox())) return;
    if (ViewInfo.GameMode) return;

    pRenderer->AddOpaqueMesh(this, -1, AABox(), eDrawMesh);

    if (mSelected)
        pRenderer->AddOpaqueMesh(this, -1, AABox(), eDrawSelection);
}

void CCollisionNode::Draw(FRenderOptions /*Options*/, int /*ComponentIndex*/, const SViewInfo& ViewInfo)
{
    if (!mpCollision) return;

    LoadModelMatrix();

    glBlendFunc(GL_ONE, GL_ZERO);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glDepthMask(GL_TRUE);

    CDrawUtil::UseCollisionShader(TintColor(ViewInfo));
    mpCollision->Draw();
    CDrawUtil::UseColorShader(CColor::skTransparentBlack);
    mpCollision->DrawWireframe();
}

SRayIntersection CCollisionNode::RayNodeIntersectTest(const CRay& /*Ray*/, u32 /*AssetID*/, const SViewInfo& /*ViewInfo*/)
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
