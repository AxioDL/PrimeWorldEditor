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

    // Objects
    std::unordered_map<u32, CScriptNode*> mScriptNodeMap;

public:
    CScene();
    ~CScene();

    // Scene Management
    CModelNode* CreateModelNode(CModel *pModel);
    CStaticNode* CreateStaticNode(CStaticModel *pModel);
    CCollisionNode* CreateCollisionNode(CCollisionMeshGroup *pMesh);
    CScriptNode* CreateScriptNode(CScriptObject *pObj);
    CLightNode* CreateLightNode(CLight *pLight);
    void DeleteNode(CSceneNode *pNode);
    void SetActiveArea(CGameArea *pArea);
    void SetActiveWorld(CWorld *pWorld);
    void PostLoad();
    void ClearScene();
    void AddSceneToRenderer(CRenderer *pRenderer, const SViewInfo& rkViewInfo);
    SRayIntersection SceneRayCast(const CRay& rkRay, const SViewInfo& rkViewInfo);
    CScriptNode* ScriptNodeByID(u32 InstanceID);
    CScriptNode* NodeForObject(CScriptObject *pObj);
    CLightNode* NodeForLight(CLight *pLight);

    // Setters/Getters
    CModel* GetActiveSkybox();
    CGameArea* GetActiveArea();

    // Static
    static FShowFlags ShowFlagsForNodeFlags(FNodeFlags NodeFlags);
    static FNodeFlags NodeFlagsForShowFlags(FShowFlags ShowFlags);
};

#endif // CSCENE_H
