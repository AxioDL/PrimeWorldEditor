#ifndef CSCRIPTNODE_H
#define CSCRIPTNODE_H

#include "CSceneNode.h"
#include "CModelNode.h"
#include "CCollisionNode.h"
#include "Core/Resource/Script/CScriptObject.h"
#include "Core/CLightParameters.h"

class CScriptExtra;

class CScriptNode : public CSceneNode
{
    CScriptObject *mpInstance;
    CScriptExtra *mpExtra;

    TResPtr<CModel> mpActiveModel;
    TResPtr<CTexture> mpBillboard;
    CCollisionNode *mpCollisionNode;

    bool mHasValidPosition;
    bool mHasVolumePreview;
    CModelNode *mpVolumePreviewNode;

    CLightParameters *mpLightParameters;

    enum EGameModeVisibility
    {
        eVisible, eNotVisible, eUntested
    } mGameModeVisibility;

public:
    CScriptNode(CScene *pScene, CSceneNode *pParent = 0, CScriptObject *pObject = 0);
    ENodeType NodeType();
    void PostLoad();
    void OnTransformed();
    void AddToRenderer(CRenderer *pRenderer, const SViewInfo& ViewInfo);
    void Draw(FRenderOptions Options, int ComponentIndex, const SViewInfo& ViewInfo);
    void DrawSelection();
    void RayAABoxIntersectTest(CRayCollisionTester& Tester, const SViewInfo& ViewInfo);
    SRayIntersection RayNodeIntersectTest(const CRay &Ray, u32 AssetID, const SViewInfo& ViewInfo);
    bool AllowsRotate() const;
    bool AllowsScale() const;
    bool IsVisible() const;
    CColor TintColor(const SViewInfo &ViewInfo) const;
    CColor WireframeColor() const;

    void PropertyModified(IProperty *pProp);
    void UpdatePreviewVolume();
    void GeneratePosition();
    void TestGameModeVisibility();
    CScriptObject* Object() const;
    CScriptTemplate* Template() const;
    CScriptExtra* Extra() const;
    CModel* ActiveModel() const;
    bool UsesModel() const;
    bool HasPreviewVolume() const;
    CAABox PreviewVolumeAABox() const;
    CVector2f BillboardScale() const;

protected:
    void SetActiveModel(CModel *pModel);
    void CalculateTransform(CTransform4f& rOut) const;
};

#endif // CSCRIPTNODE_H
