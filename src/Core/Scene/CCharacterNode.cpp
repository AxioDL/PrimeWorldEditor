#include "CCharacterNode.h"
#include "Core/Render/CRenderer.h"
#include <Common/CTimer.h>

CCharacterNode::CCharacterNode(CScene *pScene, u32 NodeID, CAnimSet *pChar /*= 0*/, CSceneNode *pParent /*= 0*/)
    : CSceneNode(pScene, NodeID, pParent)
    , mAnimated(true)
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
    // todo: frustum check. Currently don't have a means of pulling the AABox for the
    // current animation so this isn't in yet.
    if (!mpCharacter) return;
    UpdateTransformData();

    CModel *pModel = mpCharacter->NodeModel(mActiveCharSet);
    CSkeleton *pSkel = mpCharacter->NodeSkeleton(mActiveCharSet);

    if (pModel && rkViewInfo.ShowFlags.HasFlag(eShowObjectGeometry))
        AddModelToRenderer(pRenderer, pModel, 0);

    if (pSkel)
    {
        if (rkViewInfo.ShowFlags.HasFlag(eShowSkeletons))
            pRenderer->AddMesh(this, 0, AABox(), false, eDrawMesh, eForeground);
    }
}

void CCharacterNode::Draw(FRenderOptions Options, int ComponentIndex, ERenderCommand Command, const SViewInfo& rkViewInfo)
{
    CSkeleton *pSkel = mpCharacter->NodeSkeleton(mActiveCharSet);

    // Draw skeleton
    if (ComponentIndex == 0)
    {
        LoadModelMatrix();
        pSkel->Draw(Options, &mTransformData);
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
        LoadModelMatrix();

        // Draw surface OR draw entire model
        if (mAnimated)
            CGraphics::LoadBoneTransforms(mTransformData);
        else
            CGraphics::LoadIdentityBoneTransforms();

        CModel *pModel = mpCharacter->NodeModel(mActiveCharSet);
        DrawModelParts(pModel, Options, 0, Command);
    }
}

SRayIntersection CCharacterNode::RayNodeIntersectTest(const CRay& rkRay, u32 /*AssetID*/, const SViewInfo& rkViewInfo)
{
    // Check for bone under ray. Doesn't check for model intersections atm
    if (mpCharacter && rkViewInfo.ShowFlags.HasFlag(eShowSkeletons))
    {
        CSkeleton *pSkel = mpCharacter->NodeSkeleton(mActiveCharSet);

        if (pSkel)
        {
            UpdateTransformData();
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

CVector3f CCharacterNode::BonePosition(u32 BoneID)
{
    UpdateTransformData();
    CSkeleton *pSkel = (mpCharacter ? mpCharacter->NodeSkeleton(mActiveCharSet) : nullptr);
    CBone *pBone = (pSkel ? pSkel->BoneByID(BoneID) : nullptr);

    CVector3f Out = AbsolutePosition();
    if (pBone) Out += pBone->TransformedPosition(mTransformData);
    return Out;
}

void CCharacterNode::SetCharSet(CAnimSet *pChar)
{
    mpCharacter = pChar;
    SetActiveChar(0);
    ConditionalSetDirty();

    if (!mpCharacter)
        mLocalAABox = CAABox::skOne;
}

void CCharacterNode::SetActiveChar(u32 CharIndex)
{
    mActiveCharSet = CharIndex;
    ConditionalSetDirty();

    if (mpCharacter)
    {
        CModel *pModel = mpCharacter->NodeModel(CharIndex);
        mTransformData.ResizeToSkeleton(mpCharacter->NodeSkeleton(CharIndex));
        mLocalAABox = pModel ? pModel->AABox() : CAABox::skZero;
        MarkTransformChanged();
    }
}

void CCharacterNode::SetActiveAnim(u32 AnimIndex)
{
    mActiveAnim = AnimIndex;
    ConditionalSetDirty();
}

// ************ PROTECTED ************
void CCharacterNode::UpdateTransformData()
{
    if (mTransformDataDirty)
    {
        CSkeleton *pSkel = mpCharacter->NodeSkeleton(mActiveCharSet);
        if (pSkel) pSkel->UpdateTransform(mTransformData, CurrentAnim(), mAnimTime, false);
        mTransformDataDirty = false;
    }
}
