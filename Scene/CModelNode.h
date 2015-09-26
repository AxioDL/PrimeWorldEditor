#ifndef CMODELNODE_H
#define CMODELNODE_H

#include "CSceneNode.h"
#include <Resource/model/CModel.h>

class CModelNode : public CSceneNode
{
    CModel *mpModel;
    CToken mModelToken;
    u32 mActiveMatSet;
    bool mLightingEnabled;
    bool mForceAlphaOn;

public:
    explicit CModelNode(CSceneManager *pScene, CSceneNode *pParent = 0, CModel *pModel = 0);

    virtual ENodeType NodeType();
    virtual void AddToRenderer(CRenderer *pRenderer);
    virtual void Draw(ERenderOptions Options);
    virtual void DrawAsset(ERenderOptions Options, u32 asset);
    virtual void RayAABoxIntersectTest(CRayCollisionTester &Tester);
    virtual SRayIntersection RayNodeIntersectTest(const CRay &Ray, u32 AssetID, ERenderOptions options);

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
