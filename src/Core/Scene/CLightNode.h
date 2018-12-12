#ifndef CLIGHTNODE_H
#define CLIGHTNODE_H

#include "CSceneNode.h"
#include "Core/Resource/CLight.h"

class CLightNode : public CSceneNode
{
    CLight *mpLight;
public:
    CLightNode(CScene *pScene, uint32 NodeID, CSceneNode *pParent = 0, CLight *Light = 0);
    ENodeType NodeType();
    void AddToRenderer(CRenderer *pRenderer, const SViewInfo& ViewInfo);
    void Draw(FRenderOptions Options, int ComponentIndex, ERenderCommand Command, const SViewInfo& ViewInfo);
    void DrawSelection();
    void RayAABoxIntersectTest(CRayCollisionTester& Tester, const SViewInfo& ViewInfo);
    SRayIntersection RayNodeIntersectTest(const CRay &Ray, uint32 AssetID, const SViewInfo& ViewInfo);
    CStructRef GetProperties() const;
    void PropertyModified(IProperty* pProperty);
    bool AllowsRotate() const { return false; }
    CLight* Light();
    CVector2f BillboardScale();

protected:
    void CalculateTransform(CTransform4f& rOut) const;
};

#endif // CLIGHTNODE_H
