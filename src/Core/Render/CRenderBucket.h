#ifndef CRENDERBUCKET_H
#define CRENDERBUCKET_H

#include "CCamera.h"
#include "CDrawUtil.h"
#include "CGraphics.h"
#include "FRenderOptions.h"
#include "SRenderablePtr.h"
#include <Common/BasicTypes.h>
#include <algorithm>
#include <vector>

class CRenderBucket
{
    bool mEnableDepthSortDebugVisualization;

    class CSubBucket
    {
        std::vector<SRenderablePtr> mRenderables;
        uint32 mEstSize;
        uint32 mSize;

    public:
        CSubBucket()
            : mEstSize(0)
            , mSize(0)
        {}

        void Add(const SRenderablePtr &rkPtr);
        void Sort(const CCamera *pkCamera, bool DebugVisualization);
        void Clear();
        void Draw(const SViewInfo& rkViewInfo);
    };

    CSubBucket mOpaqueSubBucket;
    CSubBucket mTransparentSubBucket;

public:
    CRenderBucket()
        : mEnableDepthSortDebugVisualization(false)
    {}

    void Add(const SRenderablePtr& rkPtr, bool Transparent);
    void Clear();
    void Draw(const SViewInfo& rkViewInfo);
};

#endif // CRENDERBUCKET_H
