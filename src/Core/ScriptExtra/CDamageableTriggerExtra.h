#ifndef CDAMAGEABLETRIGGEREXTRA_H
#define CDAMAGEABLETRIGGEREXTRA_H

#include "CScriptExtra.h"

class CDamageableTriggerExtra : public CScriptExtra
{
    // Render fluid planes for doors in MP1
    enum class ERenderSide
    {
        NoRender = 0x0,
        North    = 0x1,
        South    = 0x2,
        West     = 0x4,
        East     = 0x8,
        Up       = 0x10,
        Down     = 0x20
    };

    CVectorRef mPlaneSize;
    TEnumRef<ERenderSide> mRenderSide;
    CAssetRef mTextureAssets[3];

    CMaterial* mpMat;
    CTexture* mpTextures[3];
    CVector2f mCoordScale;

    float mCachedRayDistance;

public:
    explicit CDamageableTriggerExtra(CScriptObject *pInstance, CScene *pScene, CScriptNode *pParent = 0);
    ~CDamageableTriggerExtra();
    void CreateMaterial();
    void UpdatePlaneTransform();
    ERenderSide RenderSideForDirection(const CVector3f& rkDir);
    ERenderSide TransformRenderSide(ERenderSide Side);
    void OnTransformed();
    void PropertyModified(IProperty* pProperty);
    bool ShouldDrawNormalAssets();
    void AddToRenderer(CRenderer *pRenderer, const SViewInfo& rkViewInfo);
    void Draw(FRenderOptions Options, int ComponentIndex, ERenderCommand Command, const SViewInfo& rkViewInfo);
    void DrawSelection();
    void RayAABoxIntersectTest(CRayCollisionTester& rTester, const SViewInfo& rkViewInfo);
    SRayIntersection RayNodeIntersectTest(const CRay& rkRay, uint32 ComponentIndex, const SViewInfo& rkViewInfo);
};

#endif // CDAMAGEABLETRIGGEREXTRA_H
