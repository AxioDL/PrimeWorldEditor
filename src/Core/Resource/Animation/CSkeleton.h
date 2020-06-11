#ifndef CSKELETON_H
#define CSKELETON_H

#include "CAnimation.h"
#include "Core/Render/FRenderOptions.h"
#include "Core/Resource/CResource.h"
#include <Common/BasicTypes.h>
#include <Common/TString.h>
#include <Common/Math/CRay.h>
#include <Common/Math/CVector3f.h>

class CBoneTransformData;
class CBone;

struct SBoneTransformInfo
{
    CVector3f Position{CVector3f::skZero};
    CQuaternion Rotation{CQuaternion::skIdentity};
    CVector3f Scale{CVector3f::skOne};

    SBoneTransformInfo() = default;
};

class CSkeleton : public CResource
{
    DECLARE_RESOURCE_TYPE(Skeleton)
    friend class CSkeletonLoader;

    CBone *mpRootBone;
    std::vector<CBone*> mBones;

    static const float skSphereRadius;

public:
    explicit CSkeleton(CResourceEntry *pEntry = nullptr);
    ~CSkeleton() override;
    void UpdateTransform(CBoneTransformData& rData, CAnimation *pAnim, float Time, bool AnchorRoot);
    CBone* BoneByID(uint32 BoneID) const;
    CBone* BoneByName(const TString& rkBoneName) const;
    uint32 MaxBoneID() const;

    void Draw(FRenderOptions Options, const CBoneTransformData *pkData);
    std::pair<int32,float> RayIntersect(const CRay& rkRay, const CBoneTransformData& rkData);

    uint32 NumBones() const  { return mBones.size(); }
    CBone* RootBone() const  { return mpRootBone; }
};

class CBone
{
    friend class CSkeletonLoader;

    CSkeleton *mpSkeleton;
    CBone *mpParent;
    std::vector<CBone*> mChildren;
    uint32 mID;
    CVector3f mPosition;
    CVector3f mLocalPosition;
    CQuaternion mRotation;
    CQuaternion mLocalRotation;
    TString mName;
    CTransform4f mInvBind;
    bool mSelected;

public:
    CBone(CSkeleton *pSkel);
    void UpdateTransform(CBoneTransformData& rData, const SBoneTransformInfo& rkParentTransform, CAnimation *pAnim, float Time, bool AnchorRoot);
    CVector3f TransformedPosition(const CBoneTransformData& rkData) const;
    CQuaternion TransformedRotation(const CBoneTransformData& rkData) const;
    bool IsRoot() const;

    // Accessors
    inline CSkeleton* Skeleton() const                  { return mpSkeleton; }
    inline CBone* Parent() const                        { return mpParent; }
    inline uint32 NumChildren() const                   { return mChildren.size(); }
    inline CBone* ChildByIndex(uint32 Index) const      { return mChildren[Index]; }
    inline uint32 ID() const                            { return mID; }
    inline CVector3f Position() const                   { return mPosition; }
    inline CVector3f LocalPosition() const              { return mLocalPosition; }
    inline CQuaternion Rotation() const                 { return mRotation; }
    inline CQuaternion LocalRotation() const            { return mLocalRotation; }
    inline TString Name() const                         { return mName; }
    inline bool IsSelected() const                      { return mSelected; }

    inline void SetSelected(bool Selected)              { mSelected = Selected; }
};

#endif // CSKELETON_H
