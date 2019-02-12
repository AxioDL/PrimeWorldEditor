#ifndef CCOLLIDABLEOBBTREE_H
#define CCOLLIDABLEOBBTREE_H

#include "CCollisionMesh.h"
#include "SOBBTreeNode.h"

/** A collision mesh with an OBB tree for spatial queries. Represents one mesh from a DCLN file */
class CCollidableOBBTree : public CCollisionMesh
{
    friend class CCollisionLoader;
    std::unique_ptr<SOBBTreeNode> mpOBBTree;

public:
    virtual void BuildRenderData() override
    {
        if (!mRenderData.IsBuilt())
        {
            mRenderData.BuildRenderData(mIndexData);
            mRenderData.BuildBoundingHierarchyRenderData(mpOBBTree.get());
        }
    }

    /** Accessors */
    inline SOBBTreeNode* GetOBBTree() const
    {
        return mpOBBTree.get();
    }
};

#endif // CCOLLIDABLEOBBTREE_H
