#ifndef CSCENENODE_H
#define CSCENENODE_H

#include "ENodeType.h"
#include <Common/CVector3f.h>
#include <Common/CQuaternion.h>
#include <Common/CAABox.h>
#include <Common/CRay.h>
#include <Common/CRayCollisionTester.h>
#include <Common/types.h>
#include <Common/CTransform4f.h>
#include <Core/ERenderOptions.h>
#include <Resource/CLight.h>
#include <Resource/CGameArea.h>

class CRenderer;
class CSceneManager;

class CSceneNode
{
private:
    CTransform4f _mCachedTransform;
    CAABox _mCachedAABox;
    bool _mTransformOutdated;

    bool _mInheritsPosition;
    bool _mInheritsRotation;
    bool _mInheritsScale;

protected:
    static u32 smNumNodes;
    std::string mName;
    CSceneNode *mpParent;
    CSceneManager *mpScene;

    CVector3f mPosition;
    CQuaternion mRotation;
    CVector3f mScale;
    CAABox mLocalAABox;
    bool mMouseHovering;
    bool mSelected;
    bool mVisible;
    std::list<CSceneNode*> mChildren;

    u32 mLightCount;
    CLight* mLights[8];

public:
    explicit CSceneNode(CSceneManager *pScene, CSceneNode *pParent = 0);
    virtual ~CSceneNode();
    virtual ENodeType NodeType() = 0;
    virtual std::string PrefixedName() const;
    virtual void AddToRenderer(CRenderer *pRenderer) = 0;
    virtual void Draw(ERenderOptions Options) = 0;
    virtual void DrawAsset(ERenderOptions Options, u32 Asset) = 0;
    virtual void DrawSelection();
    virtual void RayAABoxIntersectTest(CRayCollisionTester& Tester);
    virtual SRayIntersection RayNodeIntersectTest(const CRay& Ray, u32 AssetID) = 0;
    virtual bool IsVisible() const;

    void Unparent();
    void RemoveChild(CSceneNode *pChild);
    void DeleteChildren();
    void SetInheritance(bool InheritPos, bool InheritRot, bool InheritScale);
    void LoadModelMatrix();
    void BuildLightList(CGameArea *pArea);
    void LoadLights();
    void DrawBoundingBox();

    // Transform
    void Translate(const CVector3f& Translation);
    void Scale(const CVector3f& Scale);
    void UpdateTransform();
    void ForceRecalculateTransform();
    void MarkTransformChanged();
    const CTransform4f& Transform();

    // Getters
    std::string Name() const;
    CSceneNode* Parent() const;
    CSceneManager* Scene();
    CVector3f GetPosition() const;
    CVector3f GetAbsolutePosition() const;
    CQuaternion GetRotation() const;
    CQuaternion GetAbsoluteRotation() const;
    CVector3f GetScale() const;
    CVector3f GetAbsoluteScale() const;
    CAABox AABox();
    CVector3f CenterPoint();
    bool MarkedVisible() const;
    bool IsMouseHovering() const;
    bool IsSelected() const;
    bool InheritsPosition() const;
    bool InheritsRotation() const;
    bool InheritsScale() const;

    // Setters
    void SetName(const std::string& Name);
    void SetMouseHovering(bool Hovering);
    void SetSelected(bool Selected);
    void SetVisible(bool Visible);

    // Static
    static int NumNodes();
};

// ************ INLINE FUNCTIONS ************
inline int CSceneNode::NumNodes()
{
    return smNumNodes;
}

#endif // CSCENENODE_H
