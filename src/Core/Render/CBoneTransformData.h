#ifndef CBONETRANSFORMDATA
#define CBONETRANSFORMDATA

#include "Core/Resource/CSkeleton.h"
#include <Common/types.h>
#include <Math/CTransform4f.h>
#include <vector>

class CBoneTransformData
{
    std::vector<CTransform4f> mBoneMatrices;

public:
    CBoneTransformData()                                        { }
    CBoneTransformData(CSkeleton *pSkel)                        { ResizeToSkeleton(pSkel); }
    inline void ResizeToSkeleton(CSkeleton *pSkel)              { mBoneMatrices.resize(pSkel ? pSkel->MaxBoneID() + 1 : 0); }
    inline CTransform4f& BoneMatrix(u32 BoneID)                 { return mBoneMatrices[BoneID]; }
    inline const CTransform4f& BoneMatrix(u32 BoneID) const     { return mBoneMatrices[BoneID]; }
    inline void* Data()                                         { return mBoneMatrices.data(); }
    inline u32 DataSize() const                                 { return mBoneMatrices.size() * sizeof(CTransform4f); }
    inline CTransform4f& operator[](u32 BoneIndex)              { return BoneMatrix(BoneIndex); }
    inline const CTransform4f& operator[](u32 BoneIndex) const  { return BoneMatrix(BoneIndex); }
};

#endif // CBONETRANSFORMDATA

