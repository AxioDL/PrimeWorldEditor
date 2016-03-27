#include "CBasicModel.h"
#include <iostream>
#include <list>

CBasicModel::CBasicModel()
    : CResource()
    , mVertexCount(0)
    , mTriangleCount(0)
    , mBuffered(false)
    , mHasOwnMaterials(false)
    , mHasOwnSurfaces(false)
{
}

CBasicModel::~CBasicModel()
{
    if (mHasOwnSurfaces)
        for (u32 iSurf = 0; iSurf < mSurfaces.size(); iSurf++)
            delete mSurfaces[iSurf];
}

u32 CBasicModel::GetVertexCount()
{
    return mVertexCount;
}

u32 CBasicModel::GetTriangleCount()
{
    return mTriangleCount;
}

CAABox CBasicModel::AABox()
{
    return mAABox;
}

bool CBasicModel::IsBuffered()
{
    return mBuffered;
}

u32 CBasicModel::GetSurfaceCount()
{
    return mSurfaces.size();
}

CAABox CBasicModel::GetSurfaceAABox(u32 Surface)
{
    return mSurfaces[Surface]->AABox;
}

SSurface* CBasicModel::GetSurface(u32 Surface)
{
    return mSurfaces[Surface];
}
