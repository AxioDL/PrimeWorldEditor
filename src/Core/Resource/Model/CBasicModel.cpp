#include "CBasicModel.h"
#include <iostream>
#include <list>

CBasicModel::CBasicModel(CResourceEntry *pEntry /*= 0*/)
    : CResource(pEntry)
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
        for (uint32 iSurf = 0; iSurf < mSurfaces.size(); iSurf++)
            delete mSurfaces[iSurf];
}

uint32 CBasicModel::GetVertexCount()
{
    return mVertexCount;
}

uint32 CBasicModel::GetTriangleCount()
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

uint32 CBasicModel::GetSurfaceCount()
{
    return mSurfaces.size();
}

CAABox CBasicModel::GetSurfaceAABox(uint32 Surface)
{
    return mSurfaces[Surface]->AABox;
}

SSurface* CBasicModel::GetSurface(uint32 Surface)
{
    return mSurfaces[Surface];
}
