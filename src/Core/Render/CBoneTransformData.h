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
    CBoneTransformData()                                            { }
    CBoneTransformData(CSkeleton *pSkel)                            { ResizeToSkeleton(pSkel); }
    inline void ResizeToSkeleton(CSkeleton *pSkel)                  { mBoneMatrices.resize(pSkel ? pSkel->MaxBoneID() + 1 : 0); }
    inline CTransform4f& BoneMatrix(uint32 BoneID)                  { return mBoneMatrices[BoneID]; }
    inline const CTransform4f& BoneMatrix(uint32 BoneID) const      { return mBoneMatrices[BoneID]; }
    inline const void* Data() const                                 { return mBoneMatrices.data(); }
    inline uint32 DataSize() const                                  { return mBoneMatrices.size() * sizeof(CTransform4f); }
    inline uint32 NumTrackedBones() const                           { return mBoneMatrices.size(); }
    inline CTransform4f& operator[](uint32 BoneIndex)               { return BoneMatrix(BoneIndex); }
    inline const CTransform4f& operator[](uint32 BoneIndex) const   { return BoneMatrix(BoneIndex); }
};

#endif // CBONETRANSFORMDATA

