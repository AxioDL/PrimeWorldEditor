#ifndef CSCENE_H
#define CSCENE_H

#include "CSceneNode.h"
#include "CRootNode.h"
#include "CLightNode.h"
#include "CModelNode.h"
#include "CScriptNode.h"
#include "CStaticNode.h"
#include "CCollisionNode.h"
#include "FShowFlags.h"
#include "Core/Render/CRenderer.h"
#include "Core/Render/SViewInfo.h"
#include "Core/Resource/CGameArea.h"
#include "Core/Resource/CWorld.h"
#include "Core/CAreaAttributes.h"
#include "Core/SRayIntersection.h"
#include <Common/types.h>

#include <unordered_map>
#include <vector>

class CScene
{
    friend class CSceneIterator;
    
    bool mSplitTerrain;
    bool mRanPostLoad;

    u32 mNumNodes;
    CRootNode *mpSceneRootNode;
    std::unordered_map<ENodeType, std::vector<CSceneNode*>> mNodes;

    TResPtr<CGameArea> mpArea;
    TResPtr<CWorld> mpWorld;
    CRootNode *mpAreaRootNode;

    // Environment
    std::vector<CAreaAttributes> mAreaAttributesObjects;

    // Node Management
    std::unordered_map<u32, CSceneNode*> mNodeMap;
    std::unordered_map<u32, CScriptNode*> mScriptMap;

public:
    CScene();
    ~CScene();

    // Scene Management
    bool IsNodeIDUsed(u32 ID) const;
    u32 CreateNodeID(u32 SuggestedID = -1) const;

    CModelNode* CreateModelNode(CModel *pModel, u32 NodeID = -1);
    CStaticNode* CreateStaticNode(CStaticModel *pModel, u32 NodeID = -1);
    CCollisionNode* CreateCollisionNode(CCollisionMeshGroup *pMesh, u32 NodeID = -1);
    CScriptNode* CreateScriptNode(CScriptObject *pObj, u32 NodeID = -1);
    CLightNode* CreateLightNode(CLight *pLight, u32 NodeID = -1);
    void DeleteNode(CSceneNode *pNode);
    void SetActiveArea(CGameArea *pArea);
    void SetActiveWorld(CWorld *pWorld);
    void PostLoad();
    void ClearScene();
    void AddSceneToRenderer(CRenderer *pRenderer, const SViewInfo& rkViewInfo);
    SRayIntersection SceneRayCast(const CRay& rkRay, const SViewInfo& rkViewInfo);
    CSceneNode* NodeByID(u32 NodeID);
    CScriptNode* NodeForInstanceID(u32 InstanceID);
    CScriptNode* NodeForInstance(CScriptObject *pObj);
    CLightNode* NodeForLight(CLight *pLight);
    CModel* ActiveSkybox();
    CGameArea* ActiveArea();

    // Static
    static FShowFlags ShowFlagsForNodeFlags(FNodeFlags NodeFlags);
    static FNodeFlags NodeFlagsForShowFlags(FShowFlags ShowFlags);
};

#endif // CSCENE_H
