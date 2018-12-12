#ifndef CSTATICNODE_H
#define CSTATICNODE_H

#include "CSceneNode.h"
#include "Core/Resource/Model/CStaticModel.h"

class CStaticNode : public CSceneNode
{
    CStaticModel *mpModel;

public:
    CStaticNode(CScene *pScene, uint32 NodeID, CSceneNode *pParent = 0, CStaticModel *pModel = 0);
    ENodeType NodeType();
    void PostLoad();
    void AddToRenderer(CRenderer *pRenderer, const SViewInfo& rkViewInfo);
    void Draw(FRenderOptions Options, int ComponentIndex, ERenderCommand Command, const SViewInfo& rkViewInfo);
    void DrawSelection();
    void RayAABoxIntersectTest(CRayCollisionTester& rTester, const SViewInfo& rkViewInfo);
    SRayIntersection RayNodeIntersectTest(const CRay& rkRay, uint32 AssetID, const SViewInfo& rkViewInfo);
};

#endif // CSTATICNODE_H
