#ifndef CMODELNODE_H
#define CMODELNODE_H

#include "CSceneNode.h"
#include "Core/Resource/Model/CModel.h"

class CModelNode : public CSceneNode
{
    TResPtr<CModel> mpModel;
    u32 mActiveMatSet;
    bool mWorldModel;
    bool mForceAlphaOn;
    CColor mTintColor;
    bool mEnableScanOverlay;
    CColor mScanOverlayColor;

public:
    explicit CModelNode(CScene *pScene, CSceneNode *pParent = 0, CModel *pModel = 0);

    virtual ENodeType NodeType();
    virtual void PostLoad();
    virtual void AddToRenderer(CRenderer *pRenderer, const SViewInfo& ViewInfo);
    virtual void Draw(FRenderOptions Options, int ComponentIndex, const SViewInfo& ViewInfo);
    virtual void DrawSelection();
    virtual void RayAABoxIntersectTest(CRayCollisionTester& Tester, const SViewInfo& ViewInfo);
    virtual SRayIntersection RayNodeIntersectTest(const CRay &Ray, u32 AssetID, const SViewInfo& ViewInfo);
    virtual CColor TintColor(const SViewInfo& rkViewInfo) const;

    // Setters
    void SetModel(CModel *pModel);

    inline void SetMatSet(u32 MatSet)                       { mActiveMatSet = MatSet; }
    inline void SetWorldModel(bool World)                   { mWorldModel = World; }
    inline void ForceAlphaEnabled(bool Enable)              { mForceAlphaOn = Enable; }
    inline void SetTintColor(const CColor& rkTintColor)     { mTintColor = rkTintColor; }
    inline void ClearTintColor()                            { mTintColor = CColor::skWhite; }
    inline void SetScanOverlayEnabled(bool Enable)          { mEnableScanOverlay = Enable; }
    inline void SetScanOverlayColor(const CColor& rkColor)  { mScanOverlayColor = rkColor; }
    inline CModel* Model() const                            { return mpModel; }
    inline u32 MatSet() const                               { return mActiveMatSet; }
    inline bool IsWorldModel() const                        { return mWorldModel; }
    inline u32 FindMeshID() const                           { return mpModel->GetSurface(0)->MeshID; }
};

#endif // CMODELNODE_H
