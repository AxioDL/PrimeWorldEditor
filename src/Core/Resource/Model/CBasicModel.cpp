#include "CBasicModel.h"

CBasicModel::CBasicModel(CResourceEntry *pEntry)
    : CResource(pEntry)
{
}

CBasicModel::~CBasicModel()
{
    if (!mHasOwnSurfaces)
        return;

    for (auto* surface : mSurfaces)
        delete surface;
}

size_t CBasicModel::GetVertexCount() const
{
    return mVertexCount;
}

size_t CBasicModel::GetTriangleCount() const
{
    return mTriangleCount;
}

CAABox CBasicModel::AABox() const
{
    return mAABox;
}

bool CBasicModel::IsBuffered() const
{
    return mBuffered;
}

size_t CBasicModel::GetSurfaceCount() const
{
    return mSurfaces.size();
}

CAABox CBasicModel::GetSurfaceAABox(size_t Surface) const
{
    return mSurfaces[Surface]->AABox;
}

SSurface* CBasicModel::GetSurface(size_t Surface)
{
    return mSurfaces[Surface];
}
