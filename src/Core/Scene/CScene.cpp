#include "Core/Scene/CScene.h"

#include "Core/CAreaAttributes.h"
#include "Core/CRayCollisionTester.h"
#include "Core/NRangeUtils.h"
#include "Core/SRayIntersection.h"
#include "Core/GameProject/CResourceEntry.h"
#include "Core/GameProject/CResourceStore.h"
#include "Core/Render/CGraphics.h"
#include "Core/Resource/CPoiToWorld.h"
#include "Core/Resource/CWorld.h"
#include "Core/Resource/Area/CGameArea.h"
#include "Core/Resource/Factory/CAreaLoader.h"
#include "Core/Resource/Script/CScriptLayer.h"
#include "Core/Render/CRenderer.h"
#include "Core/Render/SViewInfo.h"
#include "Core/Scene/CCollisionNode.h"
#include "Core/Scene/CLightNode.h"
#include "Core/Scene/CModelNode.h"
#include "Core/Scene/CRootNode.h"
#include "Core/Scene/CSceneNode.h"
#include "Core/Scene/CScriptNode.h"
#include "Core/Scene/CStaticNode.h"

#include <Common/Log.h>
#include <Common/TString.h>
#include <Common/FileIO/CFileInStream.h>

#include <algorithm>
#include <list>
#include <string>

CScene::CScene()
    : mpSceneRootNode(std::make_unique<CRootNode>(this, UINT32_MAX, nullptr))
{
}

CScene::~CScene()
{
    ClearScene();
}

bool CScene::IsNodeIDUsed(uint32_t ID) const
{
    return mNodeMap.contains(ID);
}

uint32_t CScene::CreateNodeID(uint32_t SuggestedID) const
{
    if (SuggestedID != UINT32_MAX)
    {
        if (IsNodeIDUsed(SuggestedID))
            NLog::Error("Suggested node ID is already being used! New ID will be created.");
        else
            return SuggestedID;
    }

    uint32_t ID = 0;

    while (IsNodeIDUsed(ID))
        ID++;

    return ID;
}

CModelNode* CScene::CreateModelNode(CModel *pModel, uint32_t NodeID)
{
    if (pModel == nullptr)
        return nullptr;

    const uint32_t ID = CreateNodeID(NodeID);
    auto* pNode = new CModelNode(this, ID, mpAreaRootNode.get(), pModel);
    mNodes[ENodeType::Model].push_back(pNode);
    mNodeMap.insert_or_assign(ID, pNode);
    mNumNodes++;
    return pNode;
}

CStaticNode* CScene::CreateStaticNode(CStaticModel *pModel, uint32_t NodeID)
{
    if (pModel == nullptr)
        return nullptr;

    const uint32_t ID = CreateNodeID(NodeID);
    auto* pNode = new CStaticNode(this, ID, mpAreaRootNode.get(), pModel);
    mNodes[ENodeType::Static].push_back(pNode);
    mNodeMap.insert_or_assign(ID, pNode);
    mNumNodes++;
    return pNode;
}

CCollisionNode* CScene::CreateCollisionNode(CCollisionMeshGroup *pMesh, uint32_t NodeID)
{
    if (pMesh == nullptr)
        return nullptr;

    const uint32_t ID = CreateNodeID(NodeID);
    auto* pNode = new CCollisionNode(this, ID, mpAreaRootNode.get(), pMesh);
    mNodes[ENodeType::Collision].push_back(pNode);
    mNodeMap.insert_or_assign(ID, pNode);
    mNumNodes++;
    return pNode;
}

CScriptNode* CScene::CreateScriptNode(CScriptObject *pObj, uint32_t NodeID)
{
    if (pObj == nullptr)
        return nullptr;

    const auto ID = CreateNodeID(NodeID);
    const auto InstanceID = pObj->InstanceID();

    auto *pNode = new CScriptNode(this, ID, mpAreaRootNode.get(), pObj);
    mNodes[ENodeType::Script].push_back(pNode);
    mNodeMap.insert_or_assign(ID, pNode);
    mScriptMap.insert_or_assign(InstanceID, pNode);
    pNode->BuildLightList(mpArea);

    // AreaAttributes check
    switch (pObj->ObjectTypeID())
    {
    case 0x4E:           // MP1 AreaAttributes ID
    case FOURCC('REAA'): // MP2/MP3/DKCR AreaAttributes ID
        mAreaAttributesObjects.emplace_back(pObj);
        break;
    }

    mNumNodes++;
    return pNode;
}

CLightNode* CScene::CreateLightNode(CLight *pLight, uint32_t NodeID)
{
    if (pLight == nullptr)
        return nullptr;

    const uint32_t ID = CreateNodeID(NodeID);
    auto *pNode = new CLightNode(this, ID, mpAreaRootNode.get(), pLight);
    mNodes[ENodeType::Light].push_back(pNode);
    mNodeMap.insert_or_assign(ID, pNode);
    mNumNodes++;
    return pNode;
}

void CScene::DeleteNode(CSceneNode *pNode)
{
    const ENodeType Type = pNode->NodeType();
    auto& nodeEntry = mNodes[Type];

    const auto entryIt = std::ranges::find(nodeEntry, pNode);
    if (entryIt != nodeEntry.end())
        nodeEntry.erase(entryIt);

    if (const auto MapIt = mNodeMap.find(pNode->ID()); MapIt != mNodeMap.end())
        mNodeMap.erase(MapIt);

    if (Type == ENodeType::Script)
    {
        auto *pScript = static_cast<CScriptNode*>(pNode);

        if (const auto it = mScriptMap.find(pScript->Instance()->InstanceID()); it != mScriptMap.end())
            mScriptMap.erase(it);

        switch (pScript->Instance()->ObjectTypeID())
        {
        case 0x4E:
        case FOURCC('REAA'):
        {
            const auto it = std::ranges::find_if(mAreaAttributesObjects,
                                                 [&](const auto& obj) { return obj.Instance() == pScript->Instance(); });
            if (it != mAreaAttributesObjects.end())
                mAreaAttributesObjects.erase(it);
            break;
        }
        }
    }

    pNode->Unparent();
    delete pNode;
    mNumNodes--;
}

void CScene::SetActiveArea(CWorld *pWorld, CGameArea *pArea)
{
    // Clear existing area
    ClearScene();

    // Create nodes for new area
    mpWorld = pWorld;
    mpArea = pArea;
    mpAreaRootNode = std::make_unique<CRootNode>(this, UINT32_MAX, mpSceneRootNode.get());

    // Create static nodes
    for (const auto [idx, model] : Utils::enumerate(mpArea->StaticModels()))
    {
        CStaticNode* node = CreateStaticNode(model);
        node->SetName("Static World Model " + std::to_string(idx));
    }

    // Create model nodes
    for (const auto [idx, model] : Utils::enumerate(mpArea->TerrainModels()))
    {
        CModelNode* node = CreateModelNode(model);
        node->SetName("World Model " + std::to_string(idx));
        node->SetWorldModel(true);
    }

    CreateCollisionNode(mpArea->Collision());

    for (const auto* layer : mpArea->ScriptLayers())
    {
        const auto instances = layer->Instances();
        mNodes[ENodeType::Script].reserve(mNodes[ENodeType::Script].size() + std::ranges::size(instances));

        for (auto* instance : instances)
            CreateScriptNode(instance);
    }

    // Ensure script nodes have valid positions + build light lists
    for (auto* node : MakeNodeView(ENodeType::Script, true))
    {
        auto* pScript = static_cast<CScriptNode*>(node);
        pScript->GeneratePosition();
        pScript->BuildLightList(mpArea);
    }

    CGraphics::sAreaAmbientColor = CColor::TransparentBlack();

    for (auto& layer : pArea->LightLayers())
    {
        for (auto& light : layer)
        {
            if (light.Type() == ELightType::LocalAmbient)
                CGraphics::sAreaAmbientColor += light.Color();

            CreateLightNode(&light);
        }
    }

    mRanPostLoad = false;
    NLog::Debug("{} nodes", CSceneNode::NumNodes());
}

void CScene::LoadNeighborAreas(CWorld *pWorld, uint32_t AreaIndex)
{
    // Already loaded
    if (mpNeighborRootNode)
        return;

    mpNeighborRootNode = std::make_unique<CRootNode>(this, UINT32_MAX, nullptr);
    mNeighborsRanPostLoad = false;

    // Neighbors are loaded as standalone geometry + collision only
    for (uint32_t AttachedIdx = 0; AttachedIdx < pWorld->AreaAttachedCount(AreaIndex); AttachedIdx++)
    {
        const uint32_t NeighborIndex = pWorld->AreaAttachedID(AreaIndex, AttachedIdx);
        const CAssetID& NeighborID = pWorld->AreaResourceID(NeighborIndex);
        CResourceEntry *pEntry = pWorld->Entry()->ResourceStore()->FindEntry(NeighborID);

        if (pEntry == nullptr || !pEntry->HasCookedVersion())
        {
            NLog::Error("Failed to find neighbor area {}", NeighborIndex);
            continue;
        }

        CFileInStream File(pEntry->CookedAssetPath(), std::endian::big);

        if (!File.IsValid())
        {
            NLog::Error("Failed to open neighbor area: {}", pEntry->CookedAssetPath(true));
            continue;
        }
        
        CResourceStore *pOldStore = gpResourceStore;
        gpResourceStore = pEntry->ResourceStore();
        auto pArea = CAreaLoader::LoadMREA(File, pEntry, true);
        gpResourceStore = pOldStore;

        if (!pArea)
            continue;

        for (auto* model : pArea->StaticModels())
        {
            auto *pNode = new CStaticNode(this, UINT32_MAX, mpNeighborRootNode.get(), model);
            mNeighborNodes.push_back(pNode);
        }

        for (auto* model : pArea->TerrainModels())
        {
            auto *pNode = new CModelNode(this, UINT32_MAX, mpNeighborRootNode.get(), model);
            pNode->SetWorldModel(true);
            mNeighborNodes.push_back(pNode);
        }

        if (pArea->Collision() != nullptr)
            mNeighborNodes.push_back(new CCollisionNode(this, UINT32_MAX, mpNeighborRootNode.get(), pArea->Collision()));

        mNeighborAreas.push_back(std::move(pArea));
    }

    NLog::Debug("Loaded {} neighbor areas", mNeighborAreas.size());
}

void CScene::ClearNeighborAreas()
{
    mpNeighborRootNode.reset();
    mNeighborNodes.clear();
    mNeighborAreas.clear();
    mNeighborsRanPostLoad = false;
}

void CScene::PostLoad()
{
    mpSceneRootNode->OnLoadFinished();
    mRanPostLoad = true;
}

void CScene::ClearScene()
{
    ClearNeighborAreas();

    if (mpAreaRootNode)
    {
        mpAreaRootNode->Unparent();
        mpAreaRootNode.reset();
    }

    mNodes.clear();
    mAreaAttributesObjects.clear();
    mNodeMap.clear();
    mScriptMap.clear();
    mNumNodes = 0;

    mpArea = nullptr;
    mpWorld = nullptr;
}

void CScene::AddSceneToRenderer(CRenderer *pRenderer, const SViewInfo& rkViewInfo)
{
    // Call PostLoad the first time the scene is rendered to ensure the OpenGL context has been created before it runs.
    if (!mRanPostLoad)
        PostLoad();

    // Override show flags in game mode
    const FShowFlags ShowFlags = rkViewInfo.GameMode ? gkGameModeShowFlags : rkViewInfo.ShowFlags;
    const FNodeFlags NodeFlags = NodeFlagsForShowFlags(ShowFlags);

    for (auto* node : MakeNodeView(NodeFlags))
        node->AddToRenderer(pRenderer, rkViewInfo);
    
    if (mpNeighborRootNode && ShowFlags.HasFlag(EShowFlag::Neighbors))
    {
        if (!mNeighborsRanPostLoad)
        {
            mpNeighborRootNode->OnLoadFinished();
            mNeighborsRanPostLoad = true;
        }

        for (auto* node : mNeighborNodes)
        {
            if (NodeFlags.HasFlag(node->NodeType()) && node->IsVisible())
                node->AddToRenderer(pRenderer, rkViewInfo);
        }
    }
}

SRayIntersection CScene::SceneRayCast(const CRay& rkRay, const SViewInfo& rkViewInfo)
{
    const FShowFlags ShowFlags = rkViewInfo.GameMode ? gkGameModeShowFlags : rkViewInfo.ShowFlags;
    const FNodeFlags NodeFlags = NodeFlagsForShowFlags(ShowFlags);
    CRayCollisionTester Tester(rkRay);

    for (auto* node : MakeNodeView(NodeFlags))
        node->RayAABoxIntersectTest(Tester, rkViewInfo);

    return Tester.TestNodes(rkViewInfo);
}

CSceneNode* CScene::NodeByID(uint32_t NodeID)
{
    const auto it = mNodeMap.find(NodeID);

    if (it == mNodeMap.cend())
        return nullptr;

    return it->second;
}

CScriptNode* CScene::NodeForInstanceID(CInstanceID ID)
{
    const auto it = mScriptMap.find(ID);

    if (it == mScriptMap.cend())
        return nullptr;

    return it->second;
}

CScriptNode* CScene::NodeForInstance(const CScriptObject *pObj)
{
    return (pObj ? NodeForInstanceID(pObj->InstanceID()) : nullptr);
}

CLightNode* CScene::NodeForLight(const CLight *pLight)
{
    // Slow. Is there a better way to do this?
    std::vector<CSceneNode*>& rLights = mNodes[ENodeType::Light];

    const auto iter = std::ranges::find_if(rLights,
                                           [pLight](const auto* entry) { return static_cast<const CLightNode*>(entry)->Light() == pLight; });

    if (iter == rLights.cend())
        return nullptr;

    return static_cast<CLightNode*>(*iter);
}

CModel* CScene::ActiveSkybox()
{
    bool SkyEnabled = false;

    const auto iter = std::ranges::find_if(mAreaAttributesObjects, [&](const auto& obj) {
        const bool HasSky = obj.IsSkyEnabled();
        if (HasSky)
            SkyEnabled = true;
        return HasSky && obj.IsLayerEnabled() && obj.SkyModel() != nullptr;
    });

    if (iter != mAreaAttributesObjects.end())
        return iter->SkyModel();

    if (SkyEnabled)
        return mpWorld->DefaultSkybox();

    return nullptr;
}

CGameArea* CScene::ActiveArea()
{
    return mpArea;
}

// ************ STATIC ************
FShowFlags CScene::ShowFlagsForNodeFlags(FNodeFlags NodeFlags)
{
    FShowFlags Out;
    if (NodeFlags.HasFlag(ENodeType::Model))     Out |= EShowFlag::SplitWorld;
    if (NodeFlags.HasFlag(ENodeType::Static))    Out |= EShowFlag::MergedWorld;
    if (NodeFlags.HasFlag(ENodeType::Script))    Out |= EShowFlag::Objects;
    if (NodeFlags.HasFlag(ENodeType::Collision)) Out |= EShowFlag::WorldCollision;
    if (NodeFlags.HasFlag(ENodeType::Light))     Out |= EShowFlag::Lights;
    return Out;
}

FNodeFlags CScene::NodeFlagsForShowFlags(FShowFlags ShowFlags)
{
    FNodeFlags Out = ENodeType::Root;
    if (ShowFlags.HasFlag(EShowFlag::SplitWorld))     Out |= ENodeType::Model;
    if (ShowFlags.HasFlag(EShowFlag::MergedWorld))    Out |= ENodeType::Static;
    if (ShowFlags.HasFlag(EShowFlag::WorldCollision)) Out |= ENodeType::Collision;
    if (ShowFlags.HasFlag(EShowFlag::Objects))        Out |= ENodeType::Script | ENodeType::ScriptExtra;
    if (ShowFlags.HasFlag(EShowFlag::Lights))         Out |= ENodeType::Light;
    return Out;
}
