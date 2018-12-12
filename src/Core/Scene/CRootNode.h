#ifndef CROOTNODE_H
#define CROOTNODE_H

#include "CSceneNode.h"
#include <iostream>

// CRootNode's main purpose is to manage groups of other nodes as its children.
class CRootNode : public CSceneNode
{
public:
    explicit CRootNode(CScene *pScene, uint32 NodeID, CSceneNode *pParent = 0)
        : CSceneNode(pScene, NodeID, pParent) {}
    ~CRootNode() {}

    ENodeType NodeType()
    {
        return eRootNode;
    }

    inline void RayAABoxIntersectTest(CRayCollisionTester&, const SViewInfo&) {}

    inline SRayIntersection RayNodeIntersectTest(const CRay &, uint32, const SViewInfo&)
    {
        return SRayIntersection();
    }

    inline void DrawSelection() {}
};

#endif // CROOTNODE_H
