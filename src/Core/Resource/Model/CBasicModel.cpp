#include "CBasicModel.h"
#include <iostream>
#include <list>

CBasicModel::CBasicModel(CResourceEntry *pEntry)
    : CResource(pEntry)
{
}

CBasicModel::~CBasicModel()
{
    if (mHasOwnSurfaces)
        for (uint32 iSurf = 0; iSurf < mSurfaces.size(); iSurf++)
            delete mSurfaces[iSurf];
}

uint32 CBasicModel::GetVertexCount() const
{
    return mVertexCount;
}

uint32 CBasicModel::GetTriangleCount() const
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

uint32 CBasicModel::GetSurfaceCount() const
{
    return mSurfaces.size();
}

CAABox CBasicModel::GetSurfaceAABox(uint32 Surface) const
{
    return mSurfaces[Surface]->AABox;
}

SSurface* CBasicModel::GetSurface(uint32 Surface)
{
    return mSurfaces[Surface];
}
