#ifndef CSCENENODE_H
#define CSCENENODE_H

#include "ENodeType.h"
#include "Core/Render/EDepthGroup.h"
#include "Core/Render/FRenderOptions.h"
#include "Core/Render/IRenderable.h"
#include "Core/Resource/Area/CGameArea.h"
#include "Core/Resource/CLight.h"
#include "Core/CRayCollisionTester.h"
#include <Common/types.h>
#include <Math/CAABox.h>
#include <Math/CQuaternion.h>
#include <Math/CRay.h>
#include <Math/CTransform4f.h>
#include <Math/CVector3f.h>
#include <Math/ETransformSpace.h>

class CRenderer;
class CScene;

class CSceneNode : public IRenderable
{
private:
    mutable CTransform4f _mCachedTransform;
    mutable CAABox _mCachedAABox;
    mutable bool _mTransformDirty;

    bool _mInheritsPosition;
    bool _mInheritsRotation;
    bool _mInheritsScale;

    u32 _mID;

protected:
    static u32 smNumNodes;
    TString mName;
    CSceneNode *mpParent;
    CScene *mpScene;

    CVector3f mPosition;
    CQuaternion mRotation;
    CVector3f mScale;
    CAABox mLocalAABox;

    bool mMouseHovering;
    bool mSelected;
    bool mVisible;
    std::list<CSceneNode*> mChildren;

    u32 mLightLayerIndex;
    u32 mLightCount;
    CLight* mLights[8];
    CColor mAmbientColor;

public:
    explicit CSceneNode(CScene *pScene, u32 NodeID, CSceneNode *pParent = 0);
    virtual ~CSceneNode();
    virtual ENodeType NodeType() = 0;
    virtual void PostLoad() {}
    virtual void OnTransformed() {}
    virtual void AddToRenderer(CRenderer* /*pRenderer*/, const SViewInfo& /*rkViewInfo*/) {}
    virtual void DrawSelection();
    virtual void RayAABoxIntersectTest(CRayCollisionTester& rTester, const SViewInfo& rkViewInfo);
    virtual SRayIntersection RayNodeIntersectTest(const CRay& rkRay, u32 AssetID, const SViewInfo& rkViewInfo) = 0;
    virtual bool AllowsTranslate() const { return true; }
    virtual bool AllowsRotate() const { return true; }
    virtual bool AllowsScale() const { return true; }
    virtual bool IsVisible() const;
    virtual CColor TintColor(const SViewInfo& rkViewInfo) const;
    virtual CColor WireframeColor() const;
    virtual CStructRef GetProperties() const { return CStructRef(); }
    virtual void PropertyModified(IProperty* pProperty) {}

    void OnLoadFinished();
    void Unparent();
    void RemoveChild(CSceneNode *pChild);
    void DeleteChildren();
    void SetInheritance(bool InheritPos, bool InheritRot, bool InheritScale);
    void LoadModelMatrix();
    void BuildLightList(CGameArea *pArea);
    void LoadLights(const SViewInfo& rkViewInfo);
    void AddModelToRenderer(CRenderer *pRenderer, CModel *pModel, u32 MatSet);
    void DrawModelParts(CModel *pModel, FRenderOptions Options, u32 MatSet, ERenderCommand RenderCommand);
    void DrawBoundingBox() const;
    void DrawRotationArrow() const;

    // Transform
    void Translate(const CVector3f& rkTranslation, ETransformSpace TransformSpace);
    void Rotate(const CQuaternion& rkRotation, ETransformSpace TransformSpace);
    void Rotate(const CQuaternion& rkRotation, const CVector3f& rkPivot, const CQuaternion& rkPivotRotation, ETransformSpace TransformSpace);
    void Scale(const CVector3f& rkScale);
    void Scale(const CVector3f& rkScale, const CVector3f& rkPivot);
    const CTransform4f& Transform() const;
protected:
    void MarkTransformChanged() const;
    void ForceRecalculateTransform() const;
    virtual void CalculateTransform(CTransform4f& rOut) const;

public:
    CVector3f AbsolutePosition() const;
    CQuaternion AbsoluteRotation() const;
    CVector3f AbsoluteScale() const;
    CAABox AABox() const;

    // Inline Accessors
    TString Name() const                    { return mName; }
    CSceneNode* Parent() const              { return mpParent; }
    CScene* Scene() const                   { return mpScene; }
    u32 ID() const                          { return _mID; }
    CVector3f LocalPosition() const         { return mPosition; }
    CQuaternion LocalRotation() const       { return mRotation; }
    CVector3f LocalScale() const            { return mScale; }
    CVector3f CenterPoint() const           { return AABox().Center(); }
    u32 LightLayerIndex() const             { return mLightLayerIndex; }
    bool MarkedVisible() const              { return mVisible; }
    bool IsMouseHovering() const            { return mMouseHovering; }
    bool IsSelected() const                 { return mSelected; }
    bool InheritsPosition() const           { return _mInheritsPosition; }
    bool InheritsRotation() const           { return _mInheritsRotation; }
    bool InheritsScale() const              { return _mInheritsScale; }

    // Setters
    void SetName(const TString& rkName)             { mName = rkName; }
    void SetPosition(const CVector3f& rkPosition)   { mPosition = rkPosition; MarkTransformChanged(); }
    void SetRotation(const CQuaternion& rkRotation) { mRotation = rkRotation; MarkTransformChanged(); }
    void SetRotation(const CVector3f& rkRotEuler)   { mRotation = CQuaternion::FromEuler(rkRotEuler); MarkTransformChanged(); }
    void SetScale(const CVector3f& rkScale)         { mScale = rkScale; MarkTransformChanged(); }
    void SetLightLayerIndex(u32 Index)              { mLightLayerIndex = Index; }
    void SetMouseHovering(bool Hovering)            { mMouseHovering = Hovering; }
    void SetSelected(bool Selected)                 { mSelected = Selected; }
    void SetVisible(bool Visible)                   { mVisible = Visible; }

    // Static
    inline static int NumNodes() { return smNumNodes; }
    static CColor skSelectionTint;
};

#endif // CSCENENODE_H
