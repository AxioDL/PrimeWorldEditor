#ifndef CCOLLISIONNODE_H
#define CCOLLISIONNODE_H

#include "CSceneNode.h"
#include "Core/Resource/Collision/CCollisionMeshGroup.h"

class CCollisionNode : public CSceneNode
{
    TResPtr<CCollisionMeshGroup> mpCollision;

public:
    CCollisionNode(CScene *pScene, uint32 NodeID, CSceneNode *pParent = nullptr, CCollisionMeshGroup *pCollision = nullptr);
    ENodeType NodeType() override;
    void AddToRenderer(CRenderer *pRenderer, const SViewInfo& rkViewInfo) override;
    void Draw(FRenderOptions Options, int ComponentIndex, ERenderCommand Command, const SViewInfo& rkViewInfo) override;
    void RayAABoxIntersectTest(CRayCollisionTester& rTester, const SViewInfo& rkViewInfo) override;
    SRayIntersection RayNodeIntersectTest(const CRay& rkRay, uint32 AssetID, const SViewInfo& rkViewInfo) override;
    void SetCollision(CCollisionMeshGroup *pCollision);
};

#endif // CCOLLISIONNODE_H
