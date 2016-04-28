#include "CCharacterNode.h"
#include "Core/Render/CRenderer.h"
#include <Common/CTimer.h>

CCharacterNode::CCharacterNode(CScene *pScene, u32 NodeID, CAnimSet *pChar /*= 0*/, CSceneNode *pParent /*= 0*/)
    : CSceneNode(pScene, NodeID, pParent)
    , mAnimTime(0.f)
{
    SetCharSet(pChar);
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

void CCharacterNode::AddToRenderer(CRenderer *pRenderer, const SViewInfo& rkViewInfo)
{
    if (!mpCharacter) return;
    // todo: frustum check. Currently don't have a means of pulling the AABox for the
    // current animation so this isn't in yet.

    CModel *pModel = mpCharacter->NodeModel(mActiveCharSet);
    CSkeleton *pSkel = mpCharacter->NodeSkeleton(mActiveCharSet);

    if (pModel)
    {
        if (!pModel->HasTransparency(0))
            pRenderer->AddMesh(this, -1, AABox(), false, eDrawMesh);
        else
            AddSurfacesToRenderer(pRenderer, pModel, 0, rkViewInfo, eMidground, false);
    }

    if (pSkel)
    {
        CAnimation *pAnim = mpCharacter->Animation(mActiveAnim);
        pSkel->UpdateTransform(mTransformData, pAnim, mAnimTime, false);

        if (rkViewInfo.ShowFlags.HasFlag(eShowSkeletons))
            pRenderer->AddMesh(this, -2, AABox(), false, eDrawMesh, eForeground);
    }
}

void CCharacterNode::Draw(FRenderOptions Options, int ComponentIndex, const SViewInfo& rkViewInfo)
{
    CSkeleton *pSkel = mpCharacter->NodeSkeleton(mActiveCharSet);

    // Draw skeleton
    if (ComponentIndex == -2)
    {
        pSkel->Draw(Options, mTransformData);
    }

    // Draw mesh
    else
    {
        // Set lighting
        CGraphics::SetDefaultLighting();
        CGraphics::UpdateLightBlock();
        CGraphics::sVertexBlock.COLOR0_Amb = CGraphics::skDefaultAmbientColor;
        CGraphics::sPixelBlock.LightmapMultiplier = 1.f;
        CGraphics::sPixelBlock.TevColor = CColor::skWhite;
        CGraphics::sPixelBlock.TintColor = TintColor(rkViewInfo);

        // Draw surface OR draw entire model
        CGraphics::LoadBoneTransforms(mTransformData);
        CModel *pModel = mpCharacter->NodeModel(mActiveCharSet);

        if (ComponentIndex < 0)
            pModel->Draw(Options, 0);
        else
            pModel->DrawSurface(Options, ComponentIndex, 0);
    }
}

SRayIntersection CCharacterNode::RayNodeIntersectTest(const CRay& rkRay, u32 /*AssetID*/, const SViewInfo& /*rkViewInfo*/)
{
    // Check for bone under ray. Doesn't check for model intersections atm
    if (mpCharacter)
    {
        CSkeleton *pSkel = mpCharacter->NodeSkeleton(mActiveCharSet);

        if (pSkel)
        {
            std::pair<s32,float> Hit = pSkel->RayIntersect(rkRay, mTransformData);

            if (Hit.first != -1)
            {
                SRayIntersection Intersect;
                Intersect.Hit = true;
                Intersect.ComponentIndex = Hit.first;
                Intersect.Distance = Hit.second;
                Intersect.HitPoint = rkRay.PointOnRay(Hit.second);
                Intersect.pNode = this;
                return Intersect;
            }
        }
    }

    return SRayIntersection();
}

void CCharacterNode::SetCharSet(CAnimSet *pChar)
{
    mpCharacter = pChar;
    SetActiveChar(0);

    if (!mpCharacter)
        mLocalAABox = CAABox::skOne;
}

void CCharacterNode::SetActiveChar(u32 CharIndex)
{
    mActiveCharSet = CharIndex;

    if (mpCharacter)
    {
        mTransformData.ResizeToSkeleton(mpCharacter->NodeSkeleton(CharIndex));
        mLocalAABox = mpCharacter->NodeModel(CharIndex)->AABox();
        MarkTransformChanged();
    }
}

void CCharacterNode::SetActiveAnim(u32 AnimIndex)
{
    mActiveAnim = AnimIndex;
}

void CCharacterNode::SetAnimTime(float Time)
{
    mAnimTime = Time;
}
