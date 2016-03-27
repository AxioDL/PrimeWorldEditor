#ifndef CDAMAGEABLETRIGGEREXTRA_H
#define CDAMAGEABLETRIGGEREXTRA_H

#include "CScriptExtra.h"

class CDamageableTriggerExtra : public CScriptExtra
{
    // Render fluid planes for doors in MP1
    enum ERenderSide
    {
        eNoRender = 0x0,
        eNorth    = 0x1,
        eSouth    = 0x2,
        eWest     = 0x4,
        eEast     = 0x8,
        eUp       = 0x10,
        eDown     = 0x20
    };

    TVector3Property *mpSizeProp;
    TEnumProperty *mpRenderSideProp;
    TFileProperty *mpTextureProps[3];

    CVector3f mPlaneSize;
    ERenderSide mRenderSide;
    TResPtr<CTexture> mpTextures[3];

    CMaterial *mpMat;
    CVector2f mCoordScale;

    float mCachedRayDistance;

public:
    explicit CDamageableTriggerExtra(CScriptObject *pInstance, CScene *pScene, CSceneNode *pParent = 0);
    ~CDamageableTriggerExtra();
    void CreateMaterial();
    void UpdatePlaneTransform();
    void OnTransformed();
    void PropertyModified(IProperty *pProperty);
    bool ShouldDrawNormalAssets();
    void AddToRenderer(CRenderer *pRenderer, const SViewInfo& rkViewInfo);
    void Draw(FRenderOptions Options, int ComponentIndex, const SViewInfo& rkViewInfo);
    void DrawSelection();
    void RayAABoxIntersectTest(CRayCollisionTester& rTester, const SViewInfo& rkViewInfo);
    SRayIntersection RayNodeIntersectTest(const CRay& rkRay, u32 ComponentIndex, const SViewInfo& rkViewInfo);
};

#endif // CDAMAGEABLETRIGGEREXTRA_H
