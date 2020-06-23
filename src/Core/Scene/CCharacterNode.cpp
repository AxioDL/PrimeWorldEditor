#include "CCharacterNode.h"
#include "Core/Render/CRenderer.h"
#include <Common/CTimer.h>

CCharacterNode::CCharacterNode(CScene *pScene, uint32 NodeID, CAnimSet *pChar /*= 0*/, CSceneNode *pParent /*= 0*/)
    : CSceneNode(pScene, NodeID, pParent)
    , mAnimated(true)
    , mAnimTime(0.f)
{
    SetCharSet(pChar);
}

ENodeType CCharacterNode::NodeType()
{
    return ENodeType::Character;
}

void CCharacterNode::PostLoad()
{
    if (mpCharacter == nullptr)
        return;

    for (size_t iChar = 0; iChar < mpCharacter->NumCharacters(); iChar++)
        mpCharacter->Character(iChar)->pModel->BufferGL();
}

void CCharacterNode::AddToRenderer(CRenderer *pRenderer, const SViewInfo& rkViewInfo)
{
    // todo: frustum check. Currently don't have a means of pulling the AABox for the
    // current animation so this isn't in yet.
    if (!mpCharacter) return;
    UpdateTransformData();

    CModel *pModel = mpCharacter->Character(mActiveCharSet)->pModel;
    CSkeleton *pSkel = mpCharacter->Character(mActiveCharSet)->pSkeleton;

    if (pModel && rkViewInfo.ShowFlags.HasFlag(EShowFlag::ObjectGeometry))
        AddModelToRenderer(pRenderer, pModel, 0);

    if (pSkel)
    {
        if (rkViewInfo.ShowFlags.HasFlag(EShowFlag::Skeletons))
            pRenderer->AddMesh(this, 0, AABox(), false, ERenderCommand::DrawMesh, EDepthGroup::Foreground);
    }
}

void CCharacterNode::Draw(FRenderOptions Options, int ComponentIndex, ERenderCommand Command, const SViewInfo& rkViewInfo)
{
    CSkeleton *pSkel = mpCharacter->Character(mActiveCharSet)->pSkeleton;

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
        CGraphics::sVertexBlock.COLOR0_Mat = CColor::TransparentWhite();
        CGraphics::sPixelBlock.LightmapMultiplier = 1.f;
        CGraphics::sPixelBlock.SetAllTevColors(CColor::White());
        CGraphics::sPixelBlock.TintColor = TintColor(rkViewInfo);
        LoadModelMatrix();

        // Draw surface OR draw entire model
        if (mAnimated)
            CGraphics::LoadBoneTransforms(mTransformData);
        else
            CGraphics::LoadIdentityBoneTransforms();

        CModel *pModel = mpCharacter->Character(mActiveCharSet)->pModel;
        DrawModelParts(pModel, Options, 0, Command);
    }
}

SRayIntersection CCharacterNode::RayNodeIntersectTest(const CRay& rkRay, uint32 /*AssetID*/, const SViewInfo& rkViewInfo)
{
    // Check for bone under ray. Doesn't check for model intersections atm
    if (mpCharacter && rkViewInfo.ShowFlags.HasFlag(EShowFlag::Skeletons))
    {
        CSkeleton *pSkel = mpCharacter->Character(mActiveCharSet)->pSkeleton;

        if (pSkel)
        {
            UpdateTransformData();
            std::pair<int32,float> Hit = pSkel->RayIntersect(rkRay, mTransformData);

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

CVector3f CCharacterNode::BonePosition(uint32 BoneID)
{
    UpdateTransformData();
    CSkeleton *pSkel = (mpCharacter ? mpCharacter->Character(mActiveCharSet)->pSkeleton : nullptr);
    CBone *pBone = (pSkel ? pSkel->BoneByID(BoneID) : nullptr);

    CVector3f Out = AbsolutePosition();
    if (pBone) Out += pBone->TransformedPosition(mTransformData);
    return Out;
}

void CCharacterNode::SetCharSet(CAnimSet *pChar)
{
    mpCharacter = pChar;
    SetActiveChar(0);
    SetActiveAnim(0);
    ConditionalSetDirty();

    if (!mpCharacter)
        mLocalAABox = CAABox::One();
}

void CCharacterNode::SetActiveChar(uint32 CharIndex)
{
    mActiveCharSet = CharIndex;
    ConditionalSetDirty();

    if (mpCharacter)
    {
        CModel *pModel = mpCharacter->Character(CharIndex)->pModel;
        mTransformData.ResizeToSkeleton(mpCharacter->Character(CharIndex)->pSkeleton);
        mLocalAABox = pModel ? pModel->AABox() : CAABox::Zero();
        MarkTransformChanged();
    }
}

void CCharacterNode::SetActiveAnim(uint32 AnimIndex)
{
    mActiveAnim = AnimIndex;
    ConditionalSetDirty();
}

// ************ PROTECTED ************
void CCharacterNode::UpdateTransformData()
{
    if (mTransformDataDirty)
    {
        CSkeleton *pSkel = mpCharacter->Character(mActiveCharSet)->pSkeleton;
        if (pSkel) pSkel->UpdateTransform(mTransformData, CurrentAnim(), mAnimTime, false);
        mTransformDataDirty = false;
    }
}
