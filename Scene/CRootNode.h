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

    inline void AddToRenderer(CRenderer *) {}
    inline void Draw(ERenderOptions) {}
    inline void DrawAsset(ERenderOptions, u32) {}
    inline void RayAABoxIntersectTest(CRayCollisionTester &) {}

    inline SRayIntersection RayNodeIntersectTest(const CRay &, u32, ERenderOptions) {
        return SRayIntersection(false, 0.f, nullptr, 0);
    }

    inline void DrawSelection() {}
};

#endif // CROOTNODE_H
