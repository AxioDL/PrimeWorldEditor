#ifndef CBONETRANSFORMDATA
#define CBONETRANSFORMDATA

#include "Core/Resource/Animation/CSkeleton.h"
#include <Common/BasicTypes.h>
#include <Common/Math/CTransform4f.h>
#include <vector>

class CBoneTransformData
{
    std::vector<CTransform4f> mBoneMatrices;

public:
    CBoneTransformData() = default;
    explicit CBoneTransformData(CSkeleton *pSkel)            { ResizeToSkeleton(pSkel); }
    void ResizeToSkeleton(CSkeleton *pSkel)                  { mBoneMatrices.resize(pSkel ? pSkel->MaxBoneID() + 1 : 0); }
    CTransform4f& BoneMatrix(size_t BoneID)                  { return mBoneMatrices[BoneID]; }
    const CTransform4f& BoneMatrix(size_t BoneID) const      { return mBoneMatrices[BoneID]; }
    const void* Data() const                                 { return mBoneMatrices.data(); }
    size_t DataSize() const                                  { return mBoneMatrices.size() * sizeof(CTransform4f); }
    size_t NumTrackedBones() const                           { return mBoneMatrices.size(); }
    CTransform4f& operator[](size_t BoneIndex)               { return BoneMatrix(BoneIndex); }
    const CTransform4f& operator[](size_t BoneIndex) const   { return BoneMatrix(BoneIndex); }
};

#endif // CBONETRANSFORMDATA

