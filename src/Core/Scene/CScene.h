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
#include "Core/Resource/Area/CGameArea.h"
#include "Core/Resource/CWorld.h"
#include "Core/CAreaAttributes.h"
#include "Core/SRayIntersection.h"
#include <Common/BasicTypes.h>

#include <unordered_map>
#include <vector>

/** Needs lots of changes, see CSceneNode for most of my thoughts on this */
class CScene
{
    friend class CSceneIterator;
    
    bool mSplitTerrain = true;
    bool mRanPostLoad = false;

    uint32 mNumNodes = 0;
    CRootNode *mpSceneRootNode = nullptr;
    std::unordered_map<ENodeType, std::vector<CSceneNode*>> mNodes;

    TResPtr<CGameArea> mpArea;
    TResPtr<CWorld> mpWorld;
    CRootNode *mpAreaRootNode = nullptr;

    // Environment
    std::vector<CAreaAttributes> mAreaAttributesObjects;

    // Node Management
    std::unordered_map<uint32, CSceneNode*> mNodeMap;
    std::unordered_map<uint32, CScriptNode*> mScriptMap;

public:
    CScene();
    ~CScene();

    // Scene Management
    bool IsNodeIDUsed(uint32 ID) const;
    uint32 CreateNodeID(uint32 SuggestedID = UINT32_MAX) const;

    CModelNode* CreateModelNode(CModel *pModel, uint32 NodeID = UINT32_MAX);
    CStaticNode* CreateStaticNode(CStaticModel *pModel, uint32 NodeID = UINT32_MAX);
    CCollisionNode* CreateCollisionNode(CCollisionMeshGroup *pMesh, uint32 NodeID = UINT32_MAX);
    CScriptNode* CreateScriptNode(CScriptObject *pObj, uint32 NodeID = UINT32_MAX);
    CLightNode* CreateLightNode(CLight *pLight, uint32 NodeID = UINT32_MAX);
    void DeleteNode(CSceneNode *pNode);
    void SetActiveArea(CWorld *pWorld, CGameArea *pArea);
    void PostLoad();
    void ClearScene();
    void AddSceneToRenderer(CRenderer *pRenderer, const SViewInfo& rkViewInfo);
    SRayIntersection SceneRayCast(const CRay& rkRay, const SViewInfo& rkViewInfo);
    CSceneNode* NodeByID(uint32 NodeID);
    CScriptNode* NodeForInstanceID(uint32 InstanceID);
    CScriptNode* NodeForInstance(CScriptObject *pObj);
    CLightNode* NodeForLight(CLight *pLight);
    CModel* ActiveSkybox();
    CGameArea* ActiveArea();

    // Static
    static FShowFlags ShowFlagsForNodeFlags(FNodeFlags NodeFlags);
    static FNodeFlags NodeFlagsForShowFlags(FShowFlags ShowFlags);
};

#endif // CSCENE_H
