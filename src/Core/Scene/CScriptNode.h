#ifndef CSCRIPTNODE_H
#define CSCRIPTNODE_H

#include "CSceneNode.h"
#include "CModelNode.h"
#include "CCollisionNode.h"
#include <Core/CLightParameters.h>
#include <Resource/script/CScriptObject.h>

class CScriptNode : public CSceneNode
{
    CScriptObject *mpInstance;
    class CScriptExtra *mpExtra;

    TResPtr<CModel> mpActiveModel;
    TResPtr<CTexture> mpBillboard;
    CCollisionNode *mpCollisionNode;

    bool mHasValidPosition;
    bool mHasVolumePreview;
    float mScaleMultiplier;
    CModelNode *mpVolumePreviewNode;

    CLightParameters *mpLightParameters;

public:
    CScriptNode(CSceneManager *pScene, CSceneNode *pParent = 0, CScriptObject *pObject = 0);
    ENodeType NodeType();
    void AddToRenderer(CRenderer *pRenderer, const SViewInfo& ViewInfo);
    void Draw(ERenderOptions Options, int ComponentIndex, const SViewInfo& ViewInfo);
    void DrawSelection();
    void RayAABoxIntersectTest(CRayCollisionTester& Tester, const SViewInfo& ViewInfo);
    SRayIntersection RayNodeIntersectTest(const CRay &Ray, u32 AssetID, const SViewInfo& ViewInfo);
    bool AllowsRotate() const;
    bool AllowsScale() const;
    bool IsVisible() const;
    CColor TintColor(const SViewInfo &ViewInfo) const;
    CColor WireframeColor() const;

    void GeneratePosition();
    CScriptObject* Object() const;
    CModel* ActiveModel() const;
    bool UsesModel() const;
    bool HasPreviewVolume() const;
    CAABox PreviewVolumeAABox() const;
    CVector2f BillboardScale() const;

protected:
    void CalculateTransform(CTransform4f& rOut) const;
};

#endif // CSCRIPTNODE_H
