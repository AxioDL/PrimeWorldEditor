#ifndef CMODELNODE_H
#define CMODELNODE_H

#include "CSceneNode.h"
#include "Core/Resource/Model/CModel.h"

class CModelNode : public CSceneNode
{
    TResPtr<CModel> mpModel;
    uint32 mActiveMatSet = 0;
    bool mWorldModel = false;
    bool mForceAlphaOn = false;
    CColor mTintColor{CColor::White()};
    bool mEnableScanOverlay = false;
    CColor mScanOverlayColor;

public:
    explicit CModelNode(CScene *pScene, uint32 NodeID, CSceneNode *pParent = nullptr, CModel *pModel = nullptr);

    ENodeType NodeType() override;
    void PostLoad() override;
    void AddToRenderer(CRenderer* pRenderer, const SViewInfo& rkViewInfo) override;
    void Draw(FRenderOptions Options, int ComponentIndex, ERenderCommand Command, const SViewInfo& rkViewInfo) override;
    void DrawSelection() override;
    void RayAABoxIntersectTest(CRayCollisionTester& Tester, const SViewInfo& rkViewInfo) override;
    SRayIntersection RayNodeIntersectTest(const CRay& Ray, uint32 AssetID, const SViewInfo& rkViewInfo) override;
    CColor TintColor(const SViewInfo& rkViewInfo) const override;

    // Setters
    void SetModel(CModel *pModel);

    void SetMatSet(uint32 MatSet)                    { mActiveMatSet = MatSet; }
    void SetWorldModel(bool World)                   { mWorldModel = World; }
    void ForceAlphaEnabled(bool Enable)              { mForceAlphaOn = Enable; }
    void SetTintColor(const CColor& rkTintColor)     { mTintColor = rkTintColor; }
    void ClearTintColor()                            { mTintColor = CColor::White(); }
    void SetScanOverlayEnabled(bool Enable)          { mEnableScanOverlay = Enable; }
    void SetScanOverlayColor(const CColor& rkColor)  { mScanOverlayColor = rkColor; }
    CModel* Model() const                            { return mpModel; }
    uint32 MatSet() const                            { return mActiveMatSet; }
    bool IsWorldModel() const                        { return mWorldModel; }
    uint32 FindMeshID() const                        { return mpModel->GetSurface(0)->MeshID; }
};

#endif // CMODELNODE_H
