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
    void BuildRenderData() override;

    void BuildOBBTree();

    /** Accessors */
    SOBBTreeNode* GetOBBTree() const
    {
        return mpOBBTree.get();
    }
};

#endif // CCOLLIDABLEOBBTREE_H
