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
    void AddToRenderer(CRenderer *pRenderer);
    void Draw(ERenderOptions Options);
    void DrawAsset(ERenderOptions Options, u32 asset);
    SRayIntersection RayNodeIntersectTest(const CRay &Ray, u32 AssetID);
    CLight* Light();
};

#endif // CLIGHTNODE_H
