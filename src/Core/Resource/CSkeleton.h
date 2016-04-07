#ifndef CSKELETON_H
#define CSKELETON_H

#include "CAnimation.h"
#include "CResource.h"
#include "Core/Render/FRenderOptions.h"
#include <Common/TString.h>
#include <Common/types.h>
#include <Math/CTransform4f.h>
#include <Math/CVector3f.h>

class CSkeleton;

class CBone
{
    friend class CSkeletonLoader;

    CSkeleton *mpSkeleton;
    CBone *mpParent;
    std::vector<CBone*> mChildren;
    u32 mID;
    CVector3f mPosition;
    TString mName;

    CTransform4f mAnimTransform;
    mutable bool mAbsPosDirty;
    mutable CVector3f mAbsolutePosition;

public:
    CBone(CSkeleton *pSkel);
    void UpdateTransform(CAnimation *pAnim, float Time, bool AnchorRoot);
    bool IsRoot() const;

    // Accessors
    inline u32 ID() const                               { return mID; }
    inline CVector3f Position() const                   { return mPosition; }
    inline CBone* Parent() const                        { return mpParent; }
    inline u32 NumChildren() const                      { return mChildren.size(); }
    inline CBone* ChildByIndex(u32 Index) const         { return mChildren[Index]; }
    inline const CTransform4f& AnimTransform() const    { return mAnimTransform; }

    CVector3f AbsolutePosition() const;
};

class CSkeleton : public CResource
{
    DECLARE_RESOURCE_TYPE(eSkeleton)
    friend class CSkeletonLoader;

    CBone *mpRootBone;
    std::vector<CBone*> mBones;

public:
    CSkeleton();
    ~CSkeleton();
    void UpdateTransform(CAnimation *pAnim, float Time, bool AnchorRoot);
    CBone* BoneByID(u32 BoneID) const;
    void Draw(FRenderOptions Options);
};

#endif // CSKELETON_H
