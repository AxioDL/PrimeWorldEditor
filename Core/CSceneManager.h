#ifndef CSCENEMANAGER_h
#define CSCENEMANAGER_h

#include <string>
#include <vector>
#include <GL/glew.h>

#include "CAreaAttributes.h"
#include "CRenderer.h"
#include "SViewInfo.h"
#include <Common/SRayIntersection.h>
#include <Common/types.h>
#include <Scene/CSceneNode.h>
#include <Scene/CRootNode.h>
#include <Scene/CLightNode.h>
#include <Scene/CModelNode.h>
#include <Scene/CScriptNode.h>
#include <Scene/CStaticNode.h>
#include <Scene/CCollisionNode.h>
#include <Resource/CGameArea.h>
#include <Resource/CWorld.h>

class CSceneManager
{
    bool mSplitTerrain;

    u32 mNodeCount;
    std::vector<CModelNode*> mModelNodes;
    std::vector<CStaticNode*> mStaticNodes;
    std::vector<CCollisionNode*> mCollisionNodes;
    std::vector<CScriptNode*> mScriptNodes;
    std::vector<CLightNode*> mLightNodes;
    CRootNode *mpSceneRootNode;

    TResPtr<CGameArea> mpArea;
    TResPtr<CWorld> mpWorld;
    CRootNode *mpAreaRootNode;

    // Environment
    std::vector<CAreaAttributes> mAreaAttributesObjects;
    CAreaAttributes *mpActiveAreaAttributes;

    // Objects
    std::unordered_map<u32, CScriptNode*> mScriptNodeMap;

public:
    CSceneManager();
    ~CSceneManager();

    // Scene Management
    CModelNode* AddModel(CModel *mdl);
    CStaticNode* AddStaticModel(CStaticModel *mdl);
    CCollisionNode* AddCollision(CCollisionMeshGroup *mesh);
    CScriptNode* AddScriptObject(CScriptObject *obj);
    CLightNode* AddLight(CLight *Light);
    void SetActiveArea(CGameArea *_area);
    void SetActiveWorld(CWorld *_world);
    void ClearScene();
    void AddSceneToRenderer(CRenderer *pRenderer, const SViewInfo& ViewInfo);
    SRayIntersection SceneRayCast(const CRay& Ray, const SViewInfo& ViewInfo);
    void PickEnvironmentObjects();
    CScriptNode* ScriptNodeByID(u32 InstanceID);
    CScriptNode* NodeForObject(CScriptObject *pObj);
    CLightNode* NodeForLight(CLight *pLight);

    // Setters/Getters
    CModel* GetActiveSkybox();
    CGameArea* GetActiveArea();
};

#endif // CSCENEMANAGER_H
