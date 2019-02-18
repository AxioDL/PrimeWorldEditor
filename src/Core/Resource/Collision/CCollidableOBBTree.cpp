#include "CCollidableOBBTree.h"

void CCollidableOBBTree::BuildRenderData()
{
    if (!mRenderData.IsBuilt())
    {
        mRenderData.BuildRenderData(mIndexData);
        mRenderData.BuildBoundingHierarchyRenderData(mpOBBTree.get());
    }
}

void CCollidableOBBTree::BuildOBBTree()
{
}
