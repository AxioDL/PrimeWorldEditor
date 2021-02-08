#ifndef CSKELETON_H
#define CSKELETON_H

#include "CAnimation.h"
#include "Core/Render/FRenderOptions.h"
#include "Core/Resource/CResource.h"
#include <Common/BasicTypes.h>
#include <Common/TString.h>
#include <Common/Math/CRay.h>
#include <Common/Math/CVector3f.h>
#include <memory>
#include <string_view>

class CBoneTransformData;
class CBone;

struct SBoneTransformInfo
{
    CVector3f Position{CVector3f::Zero()};
    CQuaternion Rotation{CQuaternion::Identity()};
    CVector3f Scale{CVector3f::One()};
};

class CSkeleton : public CResource
{
    DECLARE_RESOURCE_TYPE(Skeleton)
    friend class CSkeletonLoader;

    CBone *mpRootBone = nullptr;
    std::vector<std::unique_ptr<CBone>> mBones;

    static constexpr float skSphereRadius = 0.025f;

public:
    explicit CSkeleton(CResourceEntry *pEntry = nullptr);
    ~CSkeleton() override;
    void UpdateTransform(CBoneTransformData& rData, CAnimation *pAnim, float Time, bool AnchorRoot);
    CBone* BoneByID(uint32 BoneID) const;
    CBone* BoneByName(std::string_view name) const;
    uint32 MaxBoneID() const;

    void Draw(FRenderOptions Options, const CBoneTransformData *pkData);
    std::pair<int32, float> RayIntersect(const CRay& rkRay, const CBoneTransformData& rkData) const;

    size_t NumBones() const  { return mBones.size(); }
    CBone* RootBone() const  { return mpRootBone; }
};

class CBone
{
    friend class CSkeletonLoader;

    CSkeleton *mpSkeleton;
    CBone *mpParent = nullptr;
    std::vector<CBone*> mChildren;
    uint32 mID = 0;
    CVector3f mPosition;
    CVector3f mLocalPosition;
    CQuaternion mRotation;
    CQuaternion mLocalRotation;
    TString mName;
    CTransform4f mInvBind;
    bool mSelected = false;

public:
    explicit CBone(CSkeleton *pSkel);
    void UpdateTransform(CBoneTransformData& rData, const SBoneTransformInfo& rkParentTransform, CAnimation *pAnim, float Time, bool AnchorRoot);
    CVector3f TransformedPosition(const CBoneTransformData& rkData) const;
    CQuaternion TransformedRotation(const CBoneTransformData& rkData) const;
    bool IsRoot() const;

    // Accessors
    CSkeleton* Skeleton() const                  { return mpSkeleton; }
    CBone* Parent() const                        { return mpParent; }
    size_t NumChildren() const                   { return mChildren.size(); }
    CBone* ChildByIndex(size_t Index) const      { return mChildren[Index]; }
    uint32 ID() const                            { return mID; }
    CVector3f Position() const                   { return mPosition; }
    CVector3f LocalPosition() const              { return mLocalPosition; }
    CQuaternion Rotation() const                 { return mRotation; }
    CQuaternion LocalRotation() const            { return mLocalRotation; }
    TString Name() const                         { return mName; }
    bool IsSelected() const                      { return mSelected; }

    void SetSelected(bool Selected)              { mSelected = Selected; }
};

#endif // CSKELETON_H
