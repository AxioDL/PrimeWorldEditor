#ifndef CSCENE_H
#define CSCENE_H

#include "Core/Resource/TResPtr.h"
#include "Core/Resource/Script/CInstanceID.h"
#include "Core/Scene/CSceneNode.h"
#include "Core/Scene/ENodeType.h"
#include "Core/Scene/FShowFlags.h"

#include <cstdint>
#include <memory>
#include <ranges>
#include <unordered_map>
#include <vector>

class CAreaAttributes;
class CCollisionNode;
class CCollisionMeshGroup;
class CGameArea;
class CLight;
class CLightNode;
class CModel;
class CModelNode;
class CRay;
class CRenderer;
class CRootNode;
class CScriptNode;
class CScriptObject;
class CStaticNode;
class CStaticModel;
class CWorld;

struct SRayIntersection;
struct SViewInfo;

/** Needs lots of changes, see CSceneNode for most of my thoughts on this */
class CScene
{
    bool mSplitTerrain = true;
    bool mRanPostLoad = false;

    uint32_t mNumNodes = 0;
    std::unique_ptr<CRootNode> mpSceneRootNode;
    std::unordered_map<ENodeType, std::vector<CSceneNode*>> mNodes;

    TResPtr<CGameArea> mpArea;
    TResPtr<CWorld> mpWorld;
    std::unique_ptr<CRootNode> mpAreaRootNode;

    std::vector<std::unique_ptr<CGameArea>> mNeighborAreas;
    std::unique_ptr<CRootNode> mpNeighborRootNode;
    std::vector<CSceneNode*> mNeighborNodes;
    bool mNeighborsRanPostLoad = false;

    // Environment
    std::vector<CAreaAttributes> mAreaAttributesObjects;

    // Node Management
    std::unordered_map<uint32_t, CSceneNode*> mNodeMap;
    std::unordered_map<CInstanceID, CScriptNode*> mScriptMap;

public:
    CScene();
    ~CScene();

    // Scene Management
    bool IsNodeIDUsed(uint32_t ID) const;
    uint32_t CreateNodeID(uint32_t SuggestedID = UINT32_MAX) const;

    CModelNode* CreateModelNode(CModel *pModel, uint32_t NodeID = UINT32_MAX);
    CStaticNode* CreateStaticNode(CStaticModel *pModel, uint32_t NodeID = UINT32_MAX);
    CCollisionNode* CreateCollisionNode(CCollisionMeshGroup *pMesh, uint32_t NodeID = UINT32_MAX);
    CScriptNode* CreateScriptNode(CScriptObject *pObj, uint32_t NodeID = UINT32_MAX);
    CLightNode* CreateLightNode(CLight *pLight, uint32_t NodeID = UINT32_MAX);
    void DeleteNode(CSceneNode *pNode);
    void SetActiveArea(CWorld *pWorld, CGameArea *pArea);
    void LoadNeighborAreas(CWorld *pWorld, uint32_t AreaIndex);
    void ClearNeighborAreas();
    void PostLoad();
    void ClearScene();
    void AddSceneToRenderer(CRenderer *pRenderer, const SViewInfo& rkViewInfo);
    SRayIntersection SceneRayCast(const CRay& rkRay, const SViewInfo& rkViewInfo);
    CSceneNode* NodeByID(uint32_t NodeID);
    CScriptNode* NodeForInstanceID(CInstanceID ID);
    CScriptNode* NodeForInstance(const CScriptObject *pObj);
    CLightNode* NodeForLight(const CLight *pLight);
    CModel* ActiveSkybox();
    CGameArea* ActiveArea();

    // Static
    static FShowFlags ShowFlagsForNodeFlags(FNodeFlags NodeFlags);
    static FNodeFlags NodeFlagsForShowFlags(FShowFlags ShowFlags);

    // Provides a range based view over particular scene nodes.
    auto MakeNodeView(FNodeFlags allowedNodeTypes = ENodeType::All, bool allowHiddenNodes = false) const {
        // First, check which node categories are allowed
        auto validNodes = mNodes | std::views::filter([=](const auto& entry) {
            return allowedNodeTypes.HasFlag(entry.first);
        });

        // Then create a view that joins all the allowed category vectors together without allocating
        auto nodeSequence = validNodes | std::views::values | std::views::join;

        // Now, we can lazily evaluate all of the nodes. By default we only iterate visible
        // scene nodes, but we also allow for iterating hidden nodes too
        return nodeSequence | std::views::filter([=](const auto* node) {
            return allowHiddenNodes || node->IsVisible();
        });
    }
};

#endif // CSCENE_H
