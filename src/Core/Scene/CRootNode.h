#ifndef CROOTNODE_H
#define CROOTNODE_H

#include "CSceneNode.h"
#include <iostream>

// CRootNode's main purpose is to manage groups of other nodes as its children.
class CRootNode : public CSceneNode
{
public:
    explicit CRootNode(CSceneManager *pScene, CSceneNode *pParent = 0) : CSceneNode(pScene, pParent) {}
    ~CRootNode() {}

    inline ENodeType NodeType() {
        return eRootNode;
    }

    inline void RayAABoxIntersectTest(CRayCollisionTester&, const SViewInfo&) {}

    inline SRayIntersection RayNodeIntersectTest(const CRay &, u32, const SViewInfo&) {
        return SRayIntersection(false, 0.f, nullptr, 0);
    }

    inline void DrawSelection() {}
};

#endif // CROOTNODE_H
