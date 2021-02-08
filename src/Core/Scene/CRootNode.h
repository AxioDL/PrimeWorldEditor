#ifndef CROOTNODE_H
#define CROOTNODE_H

#include "CSceneNode.h"

// CRootNode's main purpose is to manage groups of other nodes as its children.
class CRootNode : public CSceneNode
{
public:
    explicit CRootNode(CScene *pScene, uint32 NodeID, CSceneNode *pParent = nullptr)
        : CSceneNode(pScene, NodeID, pParent) {}
    ~CRootNode() override = default;

    ENodeType NodeType() override
    {
        return ENodeType::Root;
    }

    void RayAABoxIntersectTest(CRayCollisionTester&, const SViewInfo&) override {}

    SRayIntersection RayNodeIntersectTest(const CRay &, uint32, const SViewInfo&) override
    {
        return SRayIntersection();
    }

    void DrawSelection() override {}
};

#endif // CROOTNODE_H
