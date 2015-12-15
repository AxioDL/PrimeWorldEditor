#ifndef CDOOREXTRA_H
#define CDOOREXTRA_H

#include "CScriptExtra.h"

class CDoorExtra : public CScriptExtra
{
    // Render colored door shield in MP2/3
    CFileProperty *mpShieldModelProp;
    CColorProperty *mpShieldColorProp;
    TResPtr<CModel> mpShieldModel;

public:
    explicit CDoorExtra(CScriptObject *pInstance, CSceneManager *pScene, CSceneNode *pParent = 0);
    void PropertyModified(CPropertyBase *pProperty);
    void AddToRenderer(CRenderer *pRenderer, const SViewInfo& ViewInfo);
    void Draw(ERenderOptions Options, int ComponentIndex, const SViewInfo& ViewInfo);
    void DrawSelection();
    void RayAABoxIntersectTest(CRayCollisionTester& Tester, const SViewInfo& ViewInfo);
    SRayIntersection RayNodeIntersectTest(const CRay &Ray, u32 AssetID, const SViewInfo& ViewInfo);
};

#endif // CDOOREXTRA_H
