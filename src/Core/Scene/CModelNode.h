#ifndef CMODELNODE_H
#define CMODELNODE_H

#include "CSceneNode.h"
#include "Core/Resource/Model/CModel.h"

class CModelNode : public CSceneNode
{
    TResPtr<CModel> mpModel;
    uint32 mActiveMatSet;
    bool mWorldModel;
    bool mForceAlphaOn;
    CColor mTintColor;
    bool mEnableScanOverlay;
    CColor mScanOverlayColor;

public:
    explicit CModelNode(CScene *pScene, uint32 NodeID, CSceneNode *pParent = 0, CModel *pModel = 0);

    virtual ENodeType NodeType();
    virtual void PostLoad();
    virtual void AddToRenderer(CRenderer *pRenderer, const SViewInfo& rkViewInfo);
    virtual void Draw(FRenderOptions Options, int ComponentIndex, ERenderCommand Command, const SViewInfo& rkViewInfo);
    virtual void DrawSelection();
    virtual void RayAABoxIntersectTest(CRayCollisionTester& Tester, const SViewInfo& rkViewInfo);
    virtual SRayIntersection RayNodeIntersectTest(const CRay &Ray, uint32 AssetID, const SViewInfo& rkViewInfo);
    virtual CColor TintColor(const SViewInfo& rkViewInfo) const;

    // Setters
    void SetModel(CModel *pModel);

    inline void SetMatSet(uint32 MatSet)                    { mActiveMatSet = MatSet; }
    inline void SetWorldModel(bool World)                   { mWorldModel = World; }
    inline void ForceAlphaEnabled(bool Enable)              { mForceAlphaOn = Enable; }
    inline void SetTintColor(const CColor& rkTintColor)     { mTintColor = rkTintColor; }
    inline void ClearTintColor()                            { mTintColor = CColor::skWhite; }
    inline void SetScanOverlayEnabled(bool Enable)          { mEnableScanOverlay = Enable; }
    inline void SetScanOverlayColor(const CColor& rkColor)  { mScanOverlayColor = rkColor; }
    inline CModel* Model() const                            { return mpModel; }
    inline uint32 MatSet() const                            { return mActiveMatSet; }
    inline bool IsWorldModel() const                        { return mWorldModel; }
    inline uint32 FindMeshID() const                        { return mpModel->GetSurface(0)->MeshID; }
};

#endif // CMODELNODE_H
