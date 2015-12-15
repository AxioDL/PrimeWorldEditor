#ifndef CSCRIPTEXTRA_H
#define CSCRIPTEXTRA_H

#include "../CSceneNode.h"
#include "../CScriptNode.h"

/* CScriptExtra is a class that allows for additional coded behavior on any given
 * script object type. Subclass IScriptExtra, add the new class to CScriptExtra.cpp,
 * and reimplement whatever functions are needed to create the desired behavior. Note
 * that in addition to the functions here you can also reimplement IRenderable functions
 * (to render additional geometry) and CSceneNode functions (primarily for raycast
 * intersections).
 */

class CScriptExtra : public CSceneNode
{
protected:
    CScriptObject *mpInstance;
    EGame mGame;

public:
    explicit CScriptExtra(CScriptObject *pInstance, CSceneManager *pScene, CSceneNode *pParent = 0)
        : CSceneNode(pScene, pParent),
          mpInstance(pInstance),
          mGame(pInstance->Template()->Game())
    {
    }

    virtual ~CScriptExtra() {}

    // Default implementations for CSceneNode
    virtual ENodeType NodeType() { return eScriptExtraNode; }
    virtual SRayIntersection RayNodeIntersectTest(const CRay& /*Ray*/, u32 /*AssetID*/, const SViewInfo& /*ViewInfo*/)
    {
        SRayIntersection out;
        out.Hit = false;
        return out;
    }
    virtual CColor WireframeColor() const { return mpParent->WireframeColor(); }

    // Virtual CScriptExtra functions
    virtual void InstanceTransformed() {}
    virtual void PropertyModified(CPropertyBase* /*pProperty*/) {}
    virtual void LinksModified() {}
    virtual bool ShouldDrawNormalAssets() { return true; }
    virtual bool ShouldDrawVolume() { return true; }
    virtual CColor TevColor() { return CColor::skWhite; }
    virtual void ModifyTintColor(CColor& /*Color*/) {}

    // Create Script Extra
    static CScriptExtra* CreateExtra(CScriptNode *pNode);
};

#endif // CSCRIPTEXTRA_H
