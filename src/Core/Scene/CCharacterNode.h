#ifndef CCHARACTERNODE_H
#define CCHARACTERNODE_H

#include "CSceneNode.h"
#include "Core/Render/CBoneTransformData.h"
#include "Core/Resource/Animation/CAnimSet.h"

class CCharacterNode : public CSceneNode
{
    TResPtr<CAnimSet> mpCharacter;
    CBoneTransformData mTransformData;
    uint32 mActiveCharSet;
    uint32 mActiveAnim;
    bool mAnimated;
    float mAnimTime;

    mutable bool mTransformDataDirty;

public:
    explicit CCharacterNode(CScene *pScene, uint32 NodeID, CAnimSet *pChar = nullptr, CSceneNode *pParent = nullptr);

    ENodeType NodeType() override;
    void PostLoad() override;
    void AddToRenderer(CRenderer *pRenderer, const SViewInfo& rkViewInfo) override;
    void Draw(FRenderOptions Options, int ComponentIndex, ERenderCommand Command, const SViewInfo& rkViewInfo) override;
    SRayIntersection RayNodeIntersectTest(const CRay& rkRay, uint32 AssetID, const SViewInfo& rkViewInfo) override;

    CVector3f BonePosition(uint32 BoneID);
    void SetCharSet(CAnimSet *pChar);
    void SetActiveChar(uint32 CharIndex);
    void SetActiveAnim(uint32 AnimIndex);

    CAnimSet* Character() const      { return mpCharacter; }
    uint32 ActiveCharIndex() const   { return mActiveCharSet; }
    uint32 ActiveAnimIndex() const   { return mActiveAnim; }
    CAnimation* CurrentAnim() const  { return (mAnimated && mpCharacter ? mpCharacter->FindAnimationAsset(mActiveAnim) : nullptr); }
    bool IsAnimated() const          { return (mAnimated && CurrentAnim() != nullptr); }

    void SetAnimated(bool Animated)     { mAnimated = Animated; SetDirty(); }
    void SetAnimTime(float Time)        { mAnimTime = Time; ConditionalSetDirty(); }

protected:
    bool IsDirty() const         { return mTransformDataDirty; }
    void SetDirty()              { mTransformDataDirty = true; }
    void ConditionalSetDirty()   { if (IsAnimated()) SetDirty(); }
    void UpdateTransformData();
};

#endif // CCHARACTERNODE_H
