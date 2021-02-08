#ifndef CSTATICNODE_H
#define CSTATICNODE_H

#include "CSceneNode.h"
#include "Core/Resource/Model/CStaticModel.h"

class CStaticNode : public CSceneNode
{
    CStaticModel *mpModel;

public:
    CStaticNode(CScene *pScene, uint32 NodeID, CSceneNode *pParent = nullptr, CStaticModel *pModel = nullptr);
    ENodeType NodeType() override;
    void PostLoad() override;
    void AddToRenderer(CRenderer* pRenderer, const SViewInfo& rkViewInfo) override;
    void Draw(FRenderOptions Options, int ComponentIndex, ERenderCommand Command, const SViewInfo& rkViewInfo) override;
    void DrawSelection() override;
    void RayAABoxIntersectTest(CRayCollisionTester& rTester, const SViewInfo& rkViewInfo) override;
    SRayIntersection RayNodeIntersectTest(const CRay& rkRay, uint32 AssetID, const SViewInfo& rkViewInfo) override;
};

#endif // CSTATICNODE_H
