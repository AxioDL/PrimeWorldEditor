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
    explicit CDoorExtra(CScriptObject* pInstance, CScene* pScene, CScriptNode* pParent = 0);
    void PropertyModified(IProperty* pProperty);
    void AddToRenderer(CRenderer* pRenderer, const SViewInfo& rkViewInfo);
    void Draw(FRenderOptions Options, int ComponentIndex, ERenderCommand Command, const SViewInfo& rkViewInfo);
    void DrawSelection();
    void RayAABoxIntersectTest(CRayCollisionTester& rTester, const SViewInfo& rkViewInfo);
    SRayIntersection RayNodeIntersectTest(const CRay& rkRay, u32 AssetID, const SViewInfo& rkViewInfo);
};

#endif // CDOOREXTRA_H
