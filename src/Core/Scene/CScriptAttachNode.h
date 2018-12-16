#ifndef CSCRIPTATTACHNODE_H
#define CSCRIPTATTACHNODE_H

#include "CSceneNode.h"
#include "Core/Resource/Script/Property/Properties.h"
#include "Core/Resource/Script/CScriptTemplate.h"

class CScriptNode;

class CScriptAttachNode : public CSceneNode
{
    CScriptNode* mpScriptNode;
    TResPtr<CResource> mpAttachAsset;

    IProperty* mpAttachAssetProp;
    CAssetRef mAttachAssetRef;
    CAnimationSetRef mAttachAnimSetRef;

    EAttachType mAttachType;
    TString mLocatorName;
    CBone* mpLocator;

public:
    explicit CScriptAttachNode(CScene *pScene, const SAttachment& rkAttachment, CScriptNode *pParent);
    void AttachPropertyModified();
    void ParentDisplayAssetChanged(CResource *pNewDisplayAsset);
    CModel* Model() const;

    ENodeType NodeType() { return ENodeType::ScriptAttach; }
    void AddToRenderer(CRenderer *pRenderer, const SViewInfo& rkViewInfo);
    void Draw(FRenderOptions Options, int ComponentIndex, ERenderCommand Command, const SViewInfo& rkViewInfo);
    void DrawSelection();
    void RayAABoxIntersectTest(CRayCollisionTester& rTester, const SViewInfo& rkViewInfo);
    SRayIntersection RayNodeIntersectTest(const CRay& rkRay, uint32 AssetID, const SViewInfo& rkViewInfo);

    inline IProperty* AttachProperty() const { return mpAttachAssetProp; }
    inline TString LocatorName() const          { return mLocatorName; }

protected:
    void CalculateTransform(CTransform4f& rOut) const;
};

#endif // CSCRIPTATTACHNODE_H
