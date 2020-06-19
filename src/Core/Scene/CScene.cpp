#include "CScene.h"
#include "CSceneIterator.h"
#include "Core/Render/CGraphics.h"
#include "Core/Resource/CPoiToWorld.h"
#include "Core/Resource/Script/CScriptLayer.h"
#include "Core/CRayCollisionTester.h"

#include <Common/FileIO/CFileInStream.h>
#include <Common/TString.h>
#include <Common/Math/CRay.h>

#include <list>
#include <string>

CScene::CScene()
    : mpSceneRootNode(new CRootNode(this, UINT32_MAX, nullptr))
{
}

CScene::~CScene()
{
    ClearScene();
    delete mpSceneRootNode;
}

bool CScene::IsNodeIDUsed(uint32 ID) const
{
    return mNodeMap.find(ID) != mNodeMap.cend();
}

uint32 CScene::CreateNodeID(uint32 SuggestedID) const
{
    if (SuggestedID != UINT32_MAX)
    {
        if (IsNodeIDUsed(SuggestedID))
            errorf("Suggested node ID is already being used! New ID will be created.");
        else
            return SuggestedID;
    }

    uint32 ID = 0;

    while (IsNodeIDUsed(ID))
        ID++;

    return ID;
}

CModelNode* CScene::CreateModelNode(CModel *pModel, uint32 NodeID)
{
    if (pModel == nullptr)
        return nullptr;

    const uint32 ID = CreateNodeID(NodeID);
    auto* pNode = new CModelNode(this, ID, mpAreaRootNode, pModel);
    mNodes[ENodeType::Model].push_back(pNode);
    mNodeMap.insert_or_assign(ID, pNode);
    mNumNodes++;
    return pNode;
}

CStaticNode* CScene::CreateStaticNode(CStaticModel *pModel, uint32 NodeID)
{
    if (pModel == nullptr)
        return nullptr;

    const uint32 ID = CreateNodeID(NodeID);
    auto* pNode = new CStaticNode(this, ID, mpAreaRootNode, pModel);
    mNodes[ENodeType::Static].push_back(pNode);
    mNodeMap.insert_or_assign(ID, pNode);
    mNumNodes++;
    return pNode;
}

CCollisionNode* CScene::CreateCollisionNode(CCollisionMeshGroup *pMesh, uint32 NodeID)
{
    if (pMesh == nullptr)
        return nullptr;

    const uint32 ID = CreateNodeID(NodeID);
    auto* pNode = new CCollisionNode(this, ID, mpAreaRootNode, pMesh);
    mNodes[ENodeType::Collision].push_back(pNode);
    mNodeMap.insert_or_assign(ID, pNode);
    mNumNodes++;
    return pNode;
}

CScriptNode* CScene::CreateScriptNode(CScriptObject *pObj, uint32 NodeID)
{
    if (pObj == nullptr)
        return nullptr;

    const uint32 ID = CreateNodeID(NodeID);
    const uint32 InstanceID = pObj->InstanceID();

    auto *pNode = new CScriptNode(this, ID, mpAreaRootNode, pObj);
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

CLightNode* CScene::CreateLightNode(CLight *pLight, uint32 NodeID)
{
    if (pLight == nullptr)
        return nullptr;

    const uint32 ID = CreateNodeID(NodeID);
    auto *pNode = new CLightNode(this, ID, mpAreaRootNode, pLight);
    mNodes[ENodeType::Light].push_back(pNode);
    mNodeMap.insert_or_assign(ID, pNode);
    mNumNodes++;
    return pNode;
}

void CScene::DeleteNode(CSceneNode *pNode)
{
    const ENodeType Type = pNode->NodeType();
    auto& nodeEntry = mNodes[Type];

    for (auto it = nodeEntry.begin(); it != nodeEntry.end(); ++it)
    {
        if (*it == pNode)
        {
            nodeEntry.erase(it);
            break;
        }
    }

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
            for (auto it = mAreaAttributesObjects.begin(); it != mAreaAttributesObjects.end(); ++it)
            {
                if ((*it).Instance() == pScript->Instance())
                {
                    mAreaAttributesObjects.erase(it);
                    break;
                }
            }
            break;
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
    mpAreaRootNode = new CRootNode(this, UINT32_MAX, mpSceneRootNode);

    // Create static nodes
    size_t Count = mpArea->NumStaticModels();

    for (size_t iMdl = 0; iMdl < Count; iMdl++)
    {
        CStaticNode *pNode = CreateStaticNode(mpArea->StaticModel(iMdl));
        pNode->SetName("Static World Model " + std::to_string(iMdl));
    }

    // Create model nodes
    Count = mpArea->NumWorldModels();

    for (size_t iMdl = 0; iMdl < Count; iMdl++)
    {
        CModel *pModel = mpArea->TerrainModel(iMdl);
        CModelNode *pNode = CreateModelNode(pModel);
        pNode->SetName("World Model " + std::to_string(iMdl));
        pNode->SetWorldModel(true);
    }

    CreateCollisionNode(mpArea->Collision());

    const size_t NumLayers = mpArea->NumScriptLayers();

    for (size_t iLyr = 0; iLyr < NumLayers; iLyr++)
    {
        CScriptLayer *pLayer = mpArea->ScriptLayer(iLyr);
        const size_t NumObjects = pLayer->NumInstances();
        mNodes[ENodeType::Script].reserve(mNodes[ENodeType::Script].size() + NumObjects);

        for (size_t iObj = 0; iObj < NumObjects; iObj++)
        {
            CScriptObject *pObj = pLayer->InstanceByIndex(iObj);
            CreateScriptNode(pObj);
        }
    }

    // Ensure script nodes have valid positions + build light lists
    for (CSceneIterator It(this, ENodeType::Script, true); It; ++It)
    {
        CScriptNode *pScript = static_cast<CScriptNode*>(*It);
        pScript->GeneratePosition();
        pScript->BuildLightList(mpArea);
    }

    const size_t NumLightLayers = mpArea->NumLightLayers();
    CGraphics::sAreaAmbientColor = CColor::TransparentBlack();

    for (size_t iLyr = 0; iLyr < NumLightLayers; iLyr++)
    {
        const size_t NumLights = mpArea->NumLights(iLyr);

        for (size_t iLit = 0; iLit < NumLights; iLit++)
        {
            CLight *pLight = mpArea->Light(iLyr, iLit);

            if (pLight->Type() == ELightType::LocalAmbient)
                CGraphics::sAreaAmbientColor += pLight->Color();

            CreateLightNode(pLight);
        }
    }

    mRanPostLoad = false;
    debugf("%d nodes", CSceneNode::NumNodes());
}

void CScene::PostLoad()
{
    mpSceneRootNode->OnLoadFinished();
    mRanPostLoad = true;
}

void CScene::ClearScene()
{
    if (mpAreaRootNode)
    {
        mpAreaRootNode->Unparent();
        delete mpAreaRootNode;
        mpAreaRootNode = nullptr;
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

    for (CSceneIterator It(this, NodeFlags, false); It; ++It)
    {
        if (rkViewInfo.GameMode || It->IsVisible())
            It->AddToRenderer(pRenderer, rkViewInfo);
    }
}

SRayIntersection CScene::SceneRayCast(const CRay& rkRay, const SViewInfo& rkViewInfo)
{
    const FShowFlags ShowFlags = rkViewInfo.GameMode ? gkGameModeShowFlags : rkViewInfo.ShowFlags;
    const FNodeFlags NodeFlags = NodeFlagsForShowFlags(ShowFlags);
    CRayCollisionTester Tester(rkRay);

    for (CSceneIterator It(this, NodeFlags, false); It; ++It)
    {
        if (It->IsVisible())
            It->RayAABoxIntersectTest(Tester, rkViewInfo);
    }

    return Tester.TestNodes(rkViewInfo);
}

CSceneNode* CScene::NodeByID(uint32 NodeID)
{
    const auto it = mNodeMap.find(NodeID);

    if (it != mNodeMap.cend())
        return it->second;

    return nullptr;
}

CScriptNode* CScene::NodeForInstanceID(uint32 InstanceID)
{
    const auto it = mScriptMap.find(InstanceID);

    if (it != mScriptMap.cend())
        return it->second;

    return nullptr;
}

CScriptNode* CScene::NodeForInstance(CScriptObject *pObj)
{
    return (pObj ? NodeForInstanceID(pObj->InstanceID()) : nullptr);
}

CLightNode* CScene::NodeForLight(CLight *pLight)
{
    // Slow. Is there a better way to do this?
    std::vector<CSceneNode*>& rLights = mNodes[ENodeType::Light];

    const auto iter = std::find_if(rLights.begin(), rLights.end(),
                                   [pLight](const auto* entry) { return static_cast<const CLightNode*>(entry)->Light() == pLight; });

    if (iter == rLights.cend())
        return nullptr;

    return static_cast<CLightNode*>(*iter);
}

CModel* CScene::ActiveSkybox()
{
    bool SkyEnabled = false;

    for (auto& rkAttributes : mAreaAttributesObjects)
    {
        if (rkAttributes.IsSkyEnabled())
            SkyEnabled = true;

        if (rkAttributes.IsLayerEnabled())
        {
            if (rkAttributes.IsSkyEnabled())
            {
                SkyEnabled = true;
                CModel *pSky = rkAttributes.SkyModel();
                if (pSky != nullptr)
                    return pSky;
            }
        }
    }

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
    if ((NodeFlags & ENodeType::Model) != 0)     Out |= EShowFlag::SplitWorld;
    if ((NodeFlags & ENodeType::Static) != 0)    Out |= EShowFlag::MergedWorld;
    if ((NodeFlags & ENodeType::Script) != 0)    Out |= EShowFlag::Objects;
    if ((NodeFlags & ENodeType::Collision) != 0) Out |= EShowFlag::WorldCollision;
    if ((NodeFlags & ENodeType::Light) != 0)     Out |= EShowFlag::Lights;
    return Out;
}

FNodeFlags CScene::NodeFlagsForShowFlags(FShowFlags ShowFlags)
{
    FNodeFlags Out = ENodeType::Root;
    if ((ShowFlags & EShowFlag::SplitWorld) != 0)      Out |= ENodeType::Model;
    if ((ShowFlags & EShowFlag::MergedWorld) != 0)     Out |= ENodeType::Static;
    if ((ShowFlags & EShowFlag::WorldCollision) != 0)  Out |= ENodeType::Collision;
    if ((ShowFlags & EShowFlag::Objects) != 0)         Out |= ENodeType::Script | ENodeType::ScriptExtra;
    if ((ShowFlags & EShowFlag::Lights) != 0)          Out |= ENodeType::Light;
    return Out;
}
