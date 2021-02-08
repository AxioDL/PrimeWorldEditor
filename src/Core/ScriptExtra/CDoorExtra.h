#ifndef CDOOREXTRA_H
#define CDOOREXTRA_H

#include "CScriptExtra.h"

class CDoorExtra : public CScriptExtra
{
    // Render colored door shield in MP2/3
    CAssetRef mShieldModelProp;
    CColorRef mShieldColorProp;
    CBoolRef mDisabledProp;

    TResPtr<CModel> mpShieldModel;
    CColor mShieldColor;

public:
    explicit CDoorExtra(CScriptObject* pInstance, CScene* pScene, CScriptNode* pParent = nullptr);
    void PropertyModified(IProperty* pProperty) override;
    void AddToRenderer(CRenderer* pRenderer, const SViewInfo& rkViewInfo) override;
    void Draw(FRenderOptions Options, int ComponentIndex, ERenderCommand Command, const SViewInfo& rkViewInfo) override;
    void DrawSelection() override;
    void RayAABoxIntersectTest(CRayCollisionTester& rTester, const SViewInfo& rkViewInfo) override;
    SRayIntersection RayNodeIntersectTest(const CRay& rkRay, uint32 AssetID, const SViewInfo& rkViewInfo) override;
};

#endif // CDOOREXTRA_H
