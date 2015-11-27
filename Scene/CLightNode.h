#ifndef CLIGHTNODE_H
#define CLIGHTNODE_H

#include "CSceneNode.h"
#include <Resource/CLight.h>

class CLightNode : public CSceneNode
{
    CLight *mpLight;
public:
    CLightNode(CSceneManager *pScene, CSceneNode *pParent = 0, CLight *Light = 0);
    ENodeType NodeType();
    void AddToRenderer(CRenderer *pRenderer, const SViewInfo& ViewInfo);
    void Draw(ERenderOptions Options, int ComponentIndex, const SViewInfo& ViewInfo);
    void RayAABoxIntersectTest(CRayCollisionTester& Tester);
    SRayIntersection RayNodeIntersectTest(const CRay &Ray, u32 AssetID, const SViewInfo& ViewInfo);
    CLight* Light();
    CVector2f BillboardScale();
};

#endif // CLIGHTNODE_H
