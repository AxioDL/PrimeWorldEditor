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
    explicit CCharacterNode(CScene *pScene, uint32 NodeID, CAnimSet *pChar = 0, CSceneNode *pParent = 0);

    virtual ENodeType NodeType();
    virtual void PostLoad();
    virtual void AddToRenderer(CRenderer *pRenderer, const SViewInfo& rkViewInfo);
    virtual void Draw(FRenderOptions Options, int ComponentIndex, ERenderCommand Command, const SViewInfo& rkViewInfo);
    virtual SRayIntersection RayNodeIntersectTest(const CRay& rkRay, uint32 AssetID, const SViewInfo& rkViewInfo);

    CVector3f BonePosition(uint32 BoneID);
    void SetCharSet(CAnimSet *pChar);
    void SetActiveChar(uint32 CharIndex);
    void SetActiveAnim(uint32 AnimIndex);

    inline CAnimSet* Character() const      { return mpCharacter; }
    inline uint32 ActiveCharIndex() const   { return mActiveCharSet; }
    inline uint32 ActiveAnimIndex() const   { return mActiveAnim; }
    inline CAnimation* CurrentAnim() const  { return (mAnimated && mpCharacter ? mpCharacter->FindAnimationAsset(mActiveAnim) : nullptr); }
    inline bool IsAnimated() const          { return (mAnimated && CurrentAnim() != nullptr); }

    void SetAnimated(bool Animated)     { mAnimated = Animated; SetDirty(); }
    void SetAnimTime(float Time)        { mAnimTime = Time; ConditionalSetDirty(); }

protected:
    inline bool IsDirty()               { return mTransformDataDirty; }
    inline void SetDirty()              { mTransformDataDirty = true; }
    inline void ConditionalSetDirty()   { if (IsAnimated()) SetDirty(); }
    void UpdateTransformData();
};

#endif // CCHARACTERNODE_H
