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

    CModel *mpActiveModel;
    CTexture *mpBillboard;
    CToken mModelToken;
    CToken mBillboardToken;
    CCollisionNode *mpCollisionNode;

    bool mHasValidPosition;
    bool mHasVolumePreview;
    CModelNode *mpVolumePreviewNode;

    CLightParameters *mpLightParameters;

public:
    CScriptNode(CSceneManager *pScene, CSceneNode *pParent = 0, CScriptObject *pObject = 0);
    ENodeType NodeType();
    TString PrefixedName() const;
    void AddToRenderer(CRenderer *pRenderer, const SViewInfo& ViewInfo);
    void Draw(ERenderOptions Options, int ComponentIndex, const SViewInfo& ViewInfo);
    void DrawSelection();
    void RayAABoxIntersectTest(CRayCollisionTester &Tester);
    SRayIntersection RayNodeIntersectTest(const CRay &Ray, u32 AssetID, const SViewInfo& ViewInfo);
    bool IsVisible() const;
    CColor TintColor(const SViewInfo &ViewInfo) const;
    CColor WireframeColor() const;

    CScriptObject* Object();
    CModel* ActiveModel();
    void GeneratePosition();
    bool HasPreviewVolume();
    CAABox PreviewVolumeAABox();
    CVector2f BillboardScale();
};

#endif // CSCRIPTNODE_H
