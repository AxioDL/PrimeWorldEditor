#ifndef CLIGHTNODE_H
#define CLIGHTNODE_H

#include "CSceneNode.h"
#include "Core/Resource/CLight.h"

class CLightNode : public CSceneNode
{
    CLight *mpLight;
public:
    CLightNode(CScene *pScene, uint32 NodeID, CSceneNode *pParent = nullptr, CLight *Light = nullptr);
    ENodeType NodeType() override;
    void AddToRenderer(CRenderer* pRenderer, const SViewInfo& ViewInfo) override;
    void Draw(FRenderOptions Options, int ComponentIndex, ERenderCommand Command, const SViewInfo& ViewInfo) override;
    void DrawSelection() override;
    void RayAABoxIntersectTest(CRayCollisionTester& Tester, const SViewInfo& ViewInfo) override;
    SRayIntersection RayNodeIntersectTest(const CRay& Ray, uint32 AssetID, const SViewInfo& ViewInfo) override;
    CStructRef GetProperties() const override;
    void PropertyModified(IProperty* pProperty) override;
    bool AllowsRotate() const override { return false; }
    CLight* Light() { return mpLight; }
    const CLight* Light() const { return mpLight; }
    CVector2f BillboardScale() const;

protected:
    void CalculateTransform(CTransform4f& rOut) const override;
};

#endif // CLIGHTNODE_H
