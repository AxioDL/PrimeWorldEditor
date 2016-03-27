#ifndef CCOLLISIONNODE_H
#define CCOLLISIONNODE_H

#include "CSceneNode.h"
#include "Core/Resource/CCollisionMeshGroup.h"

class CCollisionNode : public CSceneNode
{
    TResPtr<CCollisionMeshGroup> mpCollision;

public:
    CCollisionNode(CScene *pScene, u32 NodeID, CSceneNode *pParent = 0, CCollisionMeshGroup *pCollision = 0);
    ENodeType NodeType();
    void AddToRenderer(CRenderer *pRenderer, const SViewInfo& rkViewInfo);
    void Draw(FRenderOptions Options, int ComponentIndex, const SViewInfo& rkViewInfo);
    SRayIntersection RayNodeIntersectTest(const CRay& rkRay, u32 AssetID, const SViewInfo& rkViewInfo);
    void SetCollision(CCollisionMeshGroup *pCollision);
};

#endif // CCOLLISIONNODE_H
