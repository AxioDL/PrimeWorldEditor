#ifndef CSTATICNODE_H
#define CSTATICNODE_H

#include "CSceneNode.h"
#include "Core/Resource/Model/CStaticModel.h"

class CStaticNode : public CSceneNode
{
    CStaticModel *mpModel;

public:
    CStaticNode(CScene *pScene, u32 NodeID, CSceneNode *pParent = 0, CStaticModel *pModel = 0);
    ENodeType NodeType();
    void PostLoad();
    void AddToRenderer(CRenderer *pRenderer, const SViewInfo& ViewInfo);
    void Draw(FRenderOptions Options, int ComponentIndex, const SViewInfo& ViewInfo);
    void DrawSelection();
    void RayAABoxIntersectTest(CRayCollisionTester& Tester, const SViewInfo& ViewInfo);
    SRayIntersection RayNodeIntersectTest(const CRay &Ray, u32 AssetID, const SViewInfo& ViewInfo);
};

#endif // CSTATICNODE_H
