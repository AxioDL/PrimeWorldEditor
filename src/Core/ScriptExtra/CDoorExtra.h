#ifndef CDOOREXTRA_H
#define CDOOREXTRA_H

#include "CScriptExtra.h"

class CDoorExtra : public CScriptExtra
{
    // Render colored door shield in MP2/3
    TFileProperty *mpShieldModelProp;
    TColorProperty *mpShieldColorProp;
    TBoolProperty *mpDisabledProp;
    TResPtr<CModel> mpShieldModel;
    CColor mShieldColor;

public:
    explicit CDoorExtra(CScriptObject *pInstance, CScene *pScene, CSceneNode *pParent = 0);
    void PropertyModified(IProperty *pProperty);
    void AddToRenderer(CRenderer *pRenderer, const SViewInfo& ViewInfo);
    void Draw(FRenderOptions Options, int ComponentIndex, const SViewInfo& ViewInfo);
    void DrawSelection();
    void RayAABoxIntersectTest(CRayCollisionTester& Tester, const SViewInfo& ViewInfo);
    SRayIntersection RayNodeIntersectTest(const CRay &Ray, u32 AssetID, const SViewInfo& ViewInfo);
};

#endif // CDOOREXTRA_H
