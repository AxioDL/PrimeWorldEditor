#ifndef CSCRIPTATTACHNODE_H
#define CSCRIPTATTACHNODE_H

#include "Core/Resource/Script/Property/Properties.h"
#include "Core/Resource/Script/CScriptTemplate.h"
#include "Core/Scene/CSceneNode.h"

class CBone;
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
    ~CScriptAttachNode() override;

    void AttachPropertyModified();
    void ParentDisplayAssetChanged(const CResource* pNewDisplayAsset);
    CModel* Model() const;

    ENodeType NodeType() const override { return ENodeType::ScriptAttach; }
    void AddToRenderer(CRenderer* pRenderer, const SViewInfo& rkViewInfo) override;
    void Draw(FRenderOptions Options, int ComponentIndex, ERenderCommand Command, const SViewInfo& rkViewInfo) override;
    void RayAABoxIntersectTest(CRayCollisionTester& rTester, const SViewInfo& rkViewInfo) override;
    SRayIntersection RayNodeIntersectTest(const CRay& rkRay, uint32_t AssetID, const SViewInfo& rkViewInfo) override;

    IProperty* AttachProperty() const  { return mpAttachAssetProp; }
    const TString& LocatorName() const { return mLocatorName; }

protected:
    void CalculateTransform(CTransform4f& rOut) const override;
};

#endif // CSCRIPTATTACHNODE_H
