#ifndef CSCENENODE_H
#define CSCENENODE_H

#include "ENodeType.h"
#include "Core/Render/EDepthGroup.h"
#include "Core/Render/FRenderOptions.h"
#include "Core/Render/IRenderable.h"
#include "Core/Resource/Area/CGameArea.h"
#include "Core/Resource/CLight.h"
#include "Core/CRayCollisionTester.h"
#include <Common/BasicTypes.h>
#include <Common/Math/CAABox.h>
#include <Common/Math/CQuaternion.h>
#include <Common/Math/CRay.h>
#include <Common/Math/CTransform4f.h>
#include <Common/Math/CVector3f.h>
#include <Common/Math/ETransformSpace.h>
#include <array>

class CRenderer;
class CScene;

/**
 * @todo so like a lot of this needs to be completely rewritten for various reasons
 * but basically currently there is a layer of abstraction between a real game object
 * and its scene representation (its node). This would normally be ok except in our
 * case there is not really any good reason for this abstraction to exist, it's just
 * an extra layer that needs to be kept in sync with the real object or else bugs
 * happen. It would make sense if we did rendering on another thread (which would give
 * us a legitimate reason to need multiple representations of an object), but we don't.
 * Splitting off into a graphics thread may be an option, but might not be worth it
 * unless it could be shown to have very significant performance benefits, considering
 * that we don't really do that much performance-intensive stuff other than rendering.
 * So barring that, the abstraction layer should just be removed entirely and objects
 * should be able to directly represent themselves in the scene, which would ultimately
 * end up simplifying quite a lot of things in the codebase. But then a lot of objects
 * like to have their own method of storing stuff like transform data (for example,
 * script objects might store it within a property data blob) so that might necessitate
 * a rewrite in how nodes store & access this kind of data in general... or maybe
 * objects shouldn't even directly have scene representations but instead their
 * primitive components (3D model, billboard, etc whatever) should.
 *
 * Another significant problem is that there is no generalized code for dealing with
 * rendering of different types of graphics priimitives (sprites, models, skinned
 * models, etc) which results in a lot of node subclasses containing duplicated code,
 * as there is no real place where this code can be centralized. There probably needs
 * to be some type of component system implemented. This is also partly a symptom of
 * a different problem where nodes simply have too much control over how they render
 * whereas the renderer doesn't have enough; it would be preferable if nodes simply
 * submitted geometry for rendering and the renderer handled it from there, rather than
 * nodes also being directly responsible for actually rendering that geometry. Also
 * nodes really really shouldn't be responsible for culling themselves; that's something
 * that really should be handled in a more generic way by the renderer.
 *
 * I'm also not a fan of the reliance on raycasting for detecting mouse input from the
 * user; the raycasting code kinda works, but it tends to be very performance-intensive
 * (especially when we currently don't have any type of octree support for the scene) and
 * requires a lot of specialized code for every type of primitive, which again gets duplicated
 * everywhere. Additionally this means you can't raycast against animated models because we do
 * all skinning on the GPU, and likewise if we had support for particles you wouldn't be able
 * to raycast against those either. So I feel it'd make sense to move to a system where we do
 * object picking via a separate rendering pass to a render target and then check which node
 * is under the mouse, which would work straight out of the box for every primitive type that
 * can be rendered, and would generally be simpler and more accurate than the raycast (which
 * in some cases can calculate different results than what object is visually under the mouse).
 *
 * Lots of text but hopefully communicates my thoughts on the current implementation and
 * what I think needs to be changed in the future.
 */
class CSceneNode : public IRenderable
{
private:
    mutable CTransform4f _mCachedTransform;
    mutable CAABox _mCachedAABox;
    mutable bool _mTransformDirty = true;

    bool _mInheritsPosition = true;
    bool _mInheritsRotation = true;
    bool _mInheritsScale = true;

    uint32 _mID;

protected:
    static uint32 smNumNodes;
    TString mName;
    CSceneNode *mpParent;
    CScene *mpScene;

    CVector3f mPosition{CVector3f::Zero()};
    CQuaternion mRotation{CQuaternion::Identity()};
    CVector3f mScale{CVector3f::One()};
    CAABox mLocalAABox;

    bool mMouseHovering = false;
    bool mSelected = false;
    bool mVisible = true;
    std::list<CSceneNode*> mChildren;

    uint32 mLightLayerIndex = 0;
    uint32 mLightCount = 0;
    std::array<CLight*, 8> mLights{};
    CColor mAmbientColor;

public:
    explicit CSceneNode(CScene *pScene, uint32 NodeID, CSceneNode *pParent = nullptr);
    ~CSceneNode() override;
    virtual ENodeType NodeType() = 0;
    virtual void PostLoad() {}
    virtual void OnTransformed() {}
    void AddToRenderer(CRenderer* /*pRenderer*/, const SViewInfo& /*rkViewInfo*/) override {}
    void DrawSelection() override;
    virtual void RayAABoxIntersectTest(CRayCollisionTester& rTester, const SViewInfo& rkViewInfo);
    virtual SRayIntersection RayNodeIntersectTest(const CRay& rkRay, uint32 AssetID, const SViewInfo& rkViewInfo) = 0;
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
    void AddModelToRenderer(CRenderer *pRenderer, CModel *pModel, size_t MatSet);
    void DrawModelParts(CModel *pModel, FRenderOptions Options, size_t MatSet, ERenderCommand RenderCommand);
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
    uint32 ID() const                       { return _mID; }
    CVector3f LocalPosition() const         { return mPosition; }
    CQuaternion LocalRotation() const       { return mRotation; }
    CVector3f LocalScale() const            { return mScale; }
    CVector3f CenterPoint() const           { return AABox().Center(); }
    uint32 LightLayerIndex() const          { return mLightLayerIndex; }
    bool MarkedVisible() const              { return mVisible; }
    bool IsMouseHovering() const            { return mMouseHovering; }
    bool IsSelected() const                 { return mSelected; }
    bool InheritsPosition() const           { return _mInheritsPosition; }
    bool InheritsRotation() const           { return _mInheritsRotation; }
    bool InheritsScale() const              { return _mInheritsScale; }

    // Setters
    void SetName(TString rkName)                    { mName = std::move(rkName); }
    void SetPosition(const CVector3f& rkPosition)   { mPosition = rkPosition; MarkTransformChanged(); }
    void SetRotation(const CQuaternion& rkRotation) { mRotation = rkRotation; MarkTransformChanged(); }
    void SetRotation(const CVector3f& rkRotEuler)   { mRotation = CQuaternion::FromEuler(rkRotEuler); MarkTransformChanged(); }
    void SetScale(const CVector3f& rkScale)         { mScale = rkScale; MarkTransformChanged(); }
    void SetLightLayerIndex(uint32 Index)           { mLightLayerIndex = Index; }
    void SetMouseHovering(bool Hovering)            { mMouseHovering = Hovering; }
    void SetSelected(bool Selected)                 { mSelected = Selected; }
    void SetVisible(bool Visible)                   { mVisible = Visible; }

    // Static
    static int NumNodes() { return smNumNodes; }
    static CColor skSelectionTint;
};

#endif // CSCENENODE_H
