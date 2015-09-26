#ifndef CSTATICNODE_H
#define CSTATICNODE_H

#include "CSceneNode.h"
#include <Resource/model/CStaticModel.h>

class CStaticNode : public CSceneNode
{
    CStaticModel *mpModel;

public:
    CStaticNode(CSceneManager *pScene, CSceneNode *pParent = 0, CStaticModel *pModel = 0);
    ENodeType NodeType();
    void AddToRenderer(CRenderer *pRenderer);
    void Draw(ERenderOptions Options);
    void DrawAsset(ERenderOptions Options, u32 asset);
    void RayAABoxIntersectTest(CRayCollisionTester &Tester);
    SRayIntersection RayNodeIntersectTest(const CRay &Ray, u32 AssetID, ERenderOptions options);
};

#endif // CSTATICNODE_H
