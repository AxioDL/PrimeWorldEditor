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
    bool mEnableDepthSortDebugVisualization = false;

    class CSubBucket
    {
        std::vector<SRenderablePtr> mRenderables;
        uint32 mEstSize = 0;
        uint32 mSize = 0;

    public:
        CSubBucket() = default;

        void Add(const SRenderablePtr &rkPtr);
        void Sort(const CCamera *pkCamera, bool DebugVisualization);
        void Clear();
        void Draw(const SViewInfo& rkViewInfo);
    };

    CSubBucket mOpaqueSubBucket;
    CSubBucket mTransparentSubBucket;

public:
    CRenderBucket() = default;

    void Add(const SRenderablePtr& rkPtr, bool Transparent);
    void Clear();
    void Draw(const SViewInfo& rkViewInfo);
};

#endif // CRENDERBUCKET_H
