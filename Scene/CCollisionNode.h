#ifndef CCOLLISIONNODE_H
#define CCOLLISIONNODE_H

#include "CSceneNode.h"
#include <Resource/CCollisionMesh.h>

class CCollisionNode : public CSceneNode
{
    CCollisionMesh *mpMesh;
    CToken mMeshToken;

public:
    CCollisionNode(CSceneManager *pScene, CSceneNode *pParent = 0, CCollisionMesh *pMesh = 0);
    ENodeType NodeType();
    void AddToRenderer(CRenderer *pRenderer);
    void Draw(ERenderOptions Options);
    void DrawAsset(ERenderOptions Options, u32 asset);
    SRayIntersection RayNodeIntersectTest(const CRay &Ray, u32 AssetID);
};

#endif // CCOLLISIONNODE_H
