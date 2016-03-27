#ifndef CRENDERBUCKET_H
#define CRENDERBUCKET_H

#include "CCamera.h"
#include "CDrawUtil.h"
#include "CGraphics.h"
#include "FRenderOptions.h"
#include "SRenderablePtr.h"
#include <Common/types.h>
#include <algorithm>
#include <vector>

class CRenderBucket
{
    bool mEnableDepthSort;
    bool mEnableDepthSortDebugVisualization;
    std::vector<SRenderablePtr> mRenderables;
    u32 mEstSize;
    u32 mSize;

public:
    CRenderBucket()
        : mEnableDepthSort(false)
        , mEnableDepthSortDebugVisualization(false)
        , mEstSize(0)
        , mSize(0)
    {}

    inline void SetDepthSortingEnabled(bool Enabled)
    {
        mEnableDepthSort = Enabled;
    }

    void Add(const SRenderablePtr& rkPtr);
    void Sort(CCamera* pCamera);
    void Clear();
    void Draw(const SViewInfo& rkViewInfo);
};

#endif // CRENDERBUCKET_H
