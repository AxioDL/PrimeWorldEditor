#include "CCollisionMesh.h"

void CCollisionMesh::BuildRenderData()
{
    if (!mRenderData.IsBuilt())
    {
        mRenderData.BuildRenderData(mIndexData);
    }
}
