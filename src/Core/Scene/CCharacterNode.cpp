#include "CCharacterNode.h"
#include <Core/Render/CRenderer.h>

CCharacterNode::CCharacterNode(CScene *pScene, u32 NodeID, CAnimSet *pChar /*= 0*/, CSceneNode *pParent /*= 0*/)
    : CSceneNode(pScene, NodeID, pParent)
{
    SetCharacter(pChar);
}

ENodeType CCharacterNode::NodeType()
{
    return eCharacterNode;
}

void CCharacterNode::PostLoad()
{
    if (mpCharacter)
    {
        for (u32 iChar = 0; iChar < mpCharacter->NumNodes(); iChar++)
            mpCharacter->NodeModel(iChar)->BufferGL();
    }
}

void CCharacterNode::AddToRenderer(CRenderer *pRenderer, const SViewInfo& /*rkViewInfo*/)
{
    if (!mpCharacter) return;

    if (mpCharacter->NodeSkeleton(mActiveCharSet))
    {
        pRenderer->AddOpaqueMesh(this, 0, AABox(), eDrawMesh);
    }
}

void CCharacterNode::Draw(FRenderOptions Options, int /*ComponentIndex*/, const SViewInfo& /*rkViewInfo*/)
{
    CSkeleton *pSkel = mpCharacter->NodeSkeleton(mActiveCharSet);
    pSkel->Draw(Options);
}

SRayIntersection CCharacterNode::RayNodeIntersectTest(const CRay& /*rkRay*/, u32 /*AssetID*/, const SViewInfo& /*rkViewInfo*/)
{
    // Not currently doing any ray checks on character nodes so don't care about this right now.
    return SRayIntersection();
}

void CCharacterNode::SetCharacter(CAnimSet *pChar)
{
    mpCharacter = pChar;
    SetActiveCharSet(0);

    if (!mpCharacter)
        mLocalAABox = CAABox::skOne;
}

void CCharacterNode::SetActiveCharSet(u32 CharIndex)
{
    mActiveCharSet = CharIndex;

    if (mpCharacter)
    {
        mLocalAABox = mpCharacter->NodeModel(CharIndex)->AABox();
        MarkTransformChanged();
    }
}
