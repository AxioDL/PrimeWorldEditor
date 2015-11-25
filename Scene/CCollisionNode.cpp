#include "CCollisionNode.h"
#include <Core/CDrawUtil.h>
#include <Core/CGraphics.h>
#include <Core/CRenderer.h>

CCollisionNode::CCollisionNode(CSceneManager *pScene, CSceneNode *pParent, CCollisionMeshGroup *pCollision)
    : CSceneNode(pScene, pParent)
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

    pRenderer->AddOpaqueMesh(this, 0, AABox(), eDrawMesh);

    if (mSelected)
        pRenderer->AddOpaqueMesh(this, 0, AABox(), eDrawSelection);
}

void CCollisionNode::Draw(ERenderOptions)
{
    // Not using parameter 1 (ERenderOptions - Options)
    if (!mpCollision) return;

    LoadModelMatrix();

    glBlendFunc(GL_ONE, GL_ZERO);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glDepthMask(GL_TRUE);

    CDrawUtil::UseCollisionShader();
    mpCollision->Draw();
    CDrawUtil::UseColorShader(CColor::skTransparentBlack);
    mpCollision->DrawWireframe();
}

void CCollisionNode::DrawAsset(ERenderOptions /*Options*/, u32 /*asset*/)
{
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
    mCollisionToken = CToken(pCollision);
}
