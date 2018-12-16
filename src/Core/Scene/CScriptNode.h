#ifndef CSCRIPTNODE_H
#define CSCRIPTNODE_H

#include "CSceneNode.h"
#include "CScriptAttachNode.h"
#include "CModelNode.h"
#include "CCollisionNode.h"
#include "Core/Resource/Script/CScriptObject.h"
#include "Core/CLightParameters.h"

class CScriptExtra;

class CScriptNode : public CSceneNode
{
    CScriptObject *mpInstance;
    CScriptExtra *mpExtra;

    TResPtr<CResource> mpDisplayAsset;
    uint32 mCharIndex;
    uint32 mAnimIndex;
    CCollisionNode *mpCollisionNode;
    std::vector<CScriptAttachNode*> mAttachments;

    bool mHasValidPosition;
    bool mHasVolumePreview;
    CModelNode *mpVolumePreviewNode;

    CLightParameters *mpLightParameters;

    enum class EGameModeVisibility
    {
        Visible, NotVisible, Untested
    } mGameModeVisibility;

public:
    CScriptNode(CScene *pScene, uint32 NodeID, CSceneNode *pParent = 0, CScriptObject *pObject = 0);
    ENodeType NodeType();
    void PostLoad();
    void OnTransformed();
    void AddToRenderer(CRenderer *pRenderer, const SViewInfo& rkViewInfo);
    void Draw(FRenderOptions Options, int ComponentIndex, ERenderCommand Command, const SViewInfo& rkViewInfo);
    void DrawSelection();
    void RayAABoxIntersectTest(CRayCollisionTester& rTester, const SViewInfo& rkViewInfo);
    SRayIntersection RayNodeIntersectTest(const CRay& rkRay, uint32 AssetID, const SViewInfo& rkViewInfo);
    bool AllowsRotate() const;
    bool AllowsScale() const;
    bool IsVisible() const;
    CColor TintColor(const SViewInfo& rkViewInfo) const;
    CColor WireframeColor() const;
    CStructRef GetProperties() const;
    void PropertyModified(IProperty* pProp);

    void LinksModified();
    void UpdatePreviewVolume();
    void GeneratePosition();
    void TestGameModeVisibility();
    CScriptObject* Instance() const;
    CScriptTemplate* Template() const;
    CScriptExtra* Extra() const;
    bool HasPreviewVolume() const;
    CAABox PreviewVolumeAABox() const;
    CVector2f BillboardScale() const;
    CTransform4f BoneTransform(uint32 BoneID, EAttachType AttachType, bool Absolute) const;

    CModel* ActiveModel() const;
    CAnimSet* ActiveAnimSet() const;
    CSkeleton* ActiveSkeleton() const;
    CAnimation* ActiveAnimation() const;
    CTexture* ActiveBillboard() const;
    bool UsesModel() const;

    inline uint32 NumAttachments() const                        { return mAttachments.size(); }
    inline CScriptAttachNode* Attachment(uint32 Index) const    { return mAttachments[Index]; }
    inline CResource* DisplayAsset() const                      { return mpDisplayAsset; }

protected:
    void SetDisplayAsset(CResource *pRes);
    void CalculateTransform(CTransform4f& rOut) const;
};

#endif // CSCRIPTNODE_H
