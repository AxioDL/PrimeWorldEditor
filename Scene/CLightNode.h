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
    void AddToRenderer(CRenderer *pRenderer, const CFrustumPlanes& frustum);
    void Draw(ERenderOptions Options);
    void DrawAsset(ERenderOptions Options, u32 asset);
    SRayIntersection RayNodeIntersectTest(const CRay &Ray, u32 AssetID, ERenderOptions options);
    CLight* Light();
};

#endif // CLIGHTNODE_H
