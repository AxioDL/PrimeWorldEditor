#ifndef CSKELETON_H
#define CSKELETON_H

#include "CResource.h"
#include "Core/Render/FRenderOptions.h"
#include <Common/TString.h>
#include <Common/types.h>
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

public:
    CBone(CSkeleton *pSkel);

    // Accessors
    inline u32 ID() const                       { return mID; }
    inline CVector3f Position() const           { return mPosition; }
    inline u32 NumChildren() const              { return mChildren.size(); }
    inline CBone* ChildByIndex(u32 Index) const { return mChildren[Index]; }
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

    CBone* BoneByID(u32 BoneID) const;
    void Draw(FRenderOptions Options);
};

#endif // CSKELETON_H
