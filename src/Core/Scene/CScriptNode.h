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
    CScriptExtra *mpExtra = nullptr;

    TResPtr<CResource> mpDisplayAsset;
    uint32 mCharIndex = 0;
    uint32 mAnimIndex = 0;
    CCollisionNode *mpCollisionNode;
    std::vector<CScriptAttachNode*> mAttachments;

    bool mHasValidPosition = false;
    bool mHasVolumePreview = false;
    CModelNode *mpVolumePreviewNode = nullptr;

    std::unique_ptr<CLightParameters> mpLightParameters;

public:
    enum class EGameModeVisibility
    {
        Visible,
        NotVisible,
        Untested,
    };
    EGameModeVisibility mGameModeVisibility{EGameModeVisibility::Untested};

    CScriptNode(CScene *pScene, uint32 NodeID, CSceneNode *pParent = nullptr, CScriptObject *pObject = nullptr);
    ENodeType NodeType() override;
    void PostLoad() override;
    void OnTransformed() override;
    void AddToRenderer(CRenderer* pRenderer, const SViewInfo& rkViewInfo) override;
    void Draw(FRenderOptions Options, int ComponentIndex, ERenderCommand Command, const SViewInfo& rkViewInfo) override;
    void DrawSelection() override;
    void RayAABoxIntersectTest(CRayCollisionTester& rTester, const SViewInfo& rkViewInfo) override;
    SRayIntersection RayNodeIntersectTest(const CRay& rkRay, uint32 AssetID, const SViewInfo& rkViewInfo) override;
    bool AllowsRotate() const override;
    bool AllowsScale() const override;
    bool IsVisible() const override;
    CColor TintColor(const SViewInfo& rkViewInfo) const override;
    CColor WireframeColor() const override;
    CStructRef GetProperties() const override;
    void PropertyModified(IProperty* pProp) override;

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

    size_t NumAttachments() const                        { return mAttachments.size(); }
    CScriptAttachNode* Attachment(size_t Index) const    { return mAttachments[Index]; }
    CResource* DisplayAsset() const                      { return mpDisplayAsset; }

protected:
    void SetDisplayAsset(CResource *pRes);
    void CalculateTransform(CTransform4f& rOut) const override;
};

#endif // CSCRIPTNODE_H
