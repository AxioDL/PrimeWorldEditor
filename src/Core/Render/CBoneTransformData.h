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
    CTransform4f& BoneMatrix(uint32 BoneID)                  { return mBoneMatrices[BoneID]; }
    const CTransform4f& BoneMatrix(uint32 BoneID) const      { return mBoneMatrices[BoneID]; }
    const void* Data() const                                 { return mBoneMatrices.data(); }
    uint32 DataSize() const                                  { return mBoneMatrices.size() * sizeof(CTransform4f); }
    uint32 NumTrackedBones() const                           { return mBoneMatrices.size(); }
    CTransform4f& operator[](uint32 BoneIndex)               { return BoneMatrix(BoneIndex); }
    const CTransform4f& operator[](uint32 BoneIndex) const   { return BoneMatrix(BoneIndex); }
};

#endif // CBONETRANSFORMDATA

