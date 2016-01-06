#ifndef CMODELNODE_H
#define CMODELNODE_H

#include "CSceneNode.h"
#include "Core/Resource/Model/CModel.h"

class CModelNode : public CSceneNode
{
    TResPtr<CModel> mpModel;
    u32 mActiveMatSet;
    bool mLightingEnabled;
    bool mForceAlphaOn;

public:
    explicit CModelNode(CScene *pScene, CSceneNode *pParent = 0, CModel *pModel = 0);

    virtual ENodeType NodeType();
    virtual void AddToRenderer(CRenderer *pRenderer, const SViewInfo& ViewInfo);
    virtual void Draw(FRenderOptions Options, int ComponentIndex, const SViewInfo& ViewInfo);
    virtual void DrawSelection();
    virtual void RayAABoxIntersectTest(CRayCollisionTester& Tester, const SViewInfo& ViewInfo);
    virtual SRayIntersection RayNodeIntersectTest(const CRay &Ray, u32 AssetID, const SViewInfo& ViewInfo);

    void SetModel(CModel *pModel);
    void SetMatSet(u32 MatSet);
    void SetDynamicLighting(bool Enable);
    void ForceAlphaEnabled(bool Enable);
    CModel* Model();
    u32 MatSet();
    bool IsDynamicLightingEnabled();
};


// ************ INLINE FUNCTIONS ************
inline void CModelNode::SetMatSet(u32 MatSet)
{
    mActiveMatSet = MatSet;
}

inline void CModelNode::SetDynamicLighting(bool Enable)
{
    mLightingEnabled = Enable;
}

inline CModel* CModelNode::Model()
{
    return mpModel;
}

inline u32 CModelNode::MatSet()
{
    return mActiveMatSet;
}

inline bool CModelNode::IsDynamicLightingEnabled()
{
    return mLightingEnabled;
}

#endif // CMODELNODE_H
