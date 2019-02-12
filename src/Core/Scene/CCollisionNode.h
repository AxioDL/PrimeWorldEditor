#ifndef CCOLLISIONNODE_H
#define CCOLLISIONNODE_H

#include "CSceneNode.h"
#include "Core/Resource/Collision/CCollisionMeshGroup.h"

class CCollisionNode : public CSceneNode
{
    TResPtr<CCollisionMeshGroup> mpCollision;

public:
    CCollisionNode(CScene *pScene, uint32 NodeID, CSceneNode *pParent = 0, CCollisionMeshGroup *pCollision = 0);
    ENodeType NodeType();
    void AddToRenderer(CRenderer *pRenderer, const SViewInfo& rkViewInfo);
    void Draw(FRenderOptions Options, int ComponentIndex, ERenderCommand Command, const SViewInfo& rkViewInfo);
    void RayAABoxIntersectTest(CRayCollisionTester& rTester, const SViewInfo& rkViewInfo);
    SRayIntersection RayNodeIntersectTest(const CRay& rkRay, uint32 AssetID, const SViewInfo& rkViewInfo);
    void SetCollision(CCollisionMeshGroup *pCollision);
};

#endif // CCOLLISIONNODE_H
