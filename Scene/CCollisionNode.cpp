#include "CCollisionNode.h"
#include <Core/CDrawUtil.h>
#include <Core/CGraphics.h>
#include <Core/CRenderer.h>

CCollisionNode::CCollisionNode(CSceneManager *pScene, CSceneNode *pParent, CCollisionMesh *pMesh)
    : CSceneNode(pScene, pParent)
{
    mpMesh = pMesh;
    mMeshToken = CToken(pMesh);
    SetName("Collision");
}

ENodeType CCollisionNode::NodeType()
{
    return eCollisionNode;
}

void CCollisionNode::AddToRenderer(CRenderer *pRenderer)
{
    if (!mpMesh) return;

    pRenderer->AddOpaqueMesh(this, 0, AABox(), eDrawMesh);

    if (mSelected)
        pRenderer->AddOpaqueMesh(this, 0, AABox(), eDrawSelection);
}

void CCollisionNode::Draw(ERenderOptions)
{
    // Not using parameter 1 (ERenderOptions - Options)
    if (!mpMesh) return;

    LoadModelMatrix();

    glBlendFunc(GL_ONE, GL_ZERO);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glDepthMask(GL_TRUE);

    CDrawUtil::UseCollisionShader();
    mpMesh->Draw();
    CDrawUtil::UseColorShader(CColor::skTransparentBlack);
    mpMesh->DrawLines();
}

void CCollisionNode::DrawAsset(ERenderOptions, u32)
{
    // Not using parameter 1 (ERenderOptions - Options)
    // Not using parameter 2 (u32 - asset)
}

SRayIntersection CCollisionNode::RayNodeIntersectTest(const CRay &Ray, u32 AssetID)
{
    // todo
    SRayIntersection Result;
    Result.Hit = false;
    return Result;
}
