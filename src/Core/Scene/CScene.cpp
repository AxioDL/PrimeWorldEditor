#include "CScene.h"
#include "Core/Render/CGraphics.h"
#include "Core/Resource/CResCache.h"
#include "Core/Resource/CPoiToWorld.h"
#include "Core/Resource/Script/CScriptLayer.h"
#include "Core/CRayCollisionTester.h"

#include <FileIO/CFileInStream.h>
#include <Common/TString.h>
#include <Math/CRay.h>

#include <list>
#include <string>

CScene::CScene()
    : mSplitTerrain(true)
    , mNumNodes(0)
    , mpSceneRootNode(new CRootNode(this, nullptr))
    , mpArea(nullptr)
    , mpWorld(nullptr)
    , mpAreaRootNode(nullptr)
{
}

CScene::~CScene()
{
    ClearScene();
}

CModelNode* CScene::CreateModelNode(CModel *pModel)
{
    if (pModel == nullptr) return nullptr;

    CModelNode *pNode = new CModelNode(this, mpSceneRootNode, pModel);
    mNodes[eModelNode].push_back(pNode);
    mNumNodes++;
    return pNode;
}

CStaticNode* CScene::CreateStaticNode(CStaticModel *pModel)
{
    if (pModel == nullptr) return nullptr;

    CStaticNode *pNode = new CStaticNode(this, mpAreaRootNode, pModel);
    mNodes[eStaticNode].push_back(pNode);
    mNumNodes++;
    return pNode;
}

CCollisionNode* CScene::CreateCollisionNode(CCollisionMeshGroup *pMesh)
{
    if (pMesh == nullptr) return nullptr;

    CCollisionNode *pNode = new CCollisionNode(this, mpAreaRootNode, pMesh);
    mNodes[eCollisionNode].push_back(pNode);
    mNumNodes++;
    return pNode;
}

CScriptNode* CScene::CreateScriptNode(CScriptObject *pObj)
{
    if (pObj == nullptr) return nullptr;

    CScriptNode *pNode = new CScriptNode(this, mpAreaRootNode, pObj);
    mNodes[eScriptNode].push_back(pNode);
    mNumNodes++;
    return pNode;
}

CLightNode* CScene::CreateLightNode(CLight *pLight)
{
    if (pLight == nullptr) return nullptr;

    CLightNode *pNode = new CLightNode(this, mpAreaRootNode, pLight);
    mNodes[eLightNode].push_back(pNode);
    mNumNodes++;
    return pNode;
}

void CScene::SetActiveArea(CGameArea *pArea)
{
    // Clear existing area
    delete mpAreaRootNode;
    mNodes.clear();
    mAreaAttributesObjects.clear();
    mScriptNodeMap.clear();

    // Create nodes for new area
    mpArea = pArea;
    mpAreaRootNode = new CRootNode(this, mpSceneRootNode);

    // Create static nodes
    u32 Count = mpArea->GetStaticModelCount();

    for (u32 iMdl = 0; iMdl < Count; iMdl++)
    {
        CStaticNode *pNode = CreateStaticNode(mpArea->GetStaticModel(iMdl));
        pNode->SetName("Static World Model " + TString::FromInt32(iMdl, 0, 10));
    }

    // Create model nodes
    Count = mpArea->GetTerrainModelCount();

    for (u32 iMdl = 0; iMdl < Count; iMdl++)
    {
        CModel *pModel = mpArea->GetTerrainModel(iMdl);
        CModelNode *pNode = CreateModelNode(pModel);
        pNode->SetName("World Model " + TString::FromInt32(iMdl, 0, 10));
        pNode->SetWorldModel(true);
    }

    CreateCollisionNode(mpArea->GetCollision());

    u32 NumLayers = mpArea->GetScriptLayerCount();

    for (u32 iLyr = 0; iLyr < NumLayers; iLyr++)
    {
        CScriptLayer *pLayer = mpArea->GetScriptLayer(iLyr);
        u32 NumObjects = pLayer->NumInstances();
        mNodes[eScriptNode].reserve(mNodes[eScriptNode].size() + NumObjects);

        for (u32 iObj = 0; iObj < NumObjects; iObj++)
        {
            CScriptObject *pObj = pLayer->InstanceByIndex(iObj);
            CScriptNode *pNode = CreateScriptNode(pObj);
            pNode->BuildLightList(mpArea);

            // Add to map
            mScriptNodeMap[pObj->InstanceID()] = pNode;

            // AreaAttributes check
            switch (pObj->ObjectTypeID())
            {
            case 0x4E:       // MP1 AreaAttributes ID
            case 0x52454141: // MP2/MP3/DKCR AreaAttributes ID ("REAA")
                mAreaAttributesObjects.emplace_back( CAreaAttributes(pObj) );
                break;
            }
        }
    }

    CScriptLayer *pGenLayer = mpArea->GetGeneratorLayer();
    if (pGenLayer)
    {
        for (u32 iObj = 0; iObj < pGenLayer->NumInstances(); iObj++)
        {
            CScriptObject *pObj = pGenLayer->InstanceByIndex(iObj);
            CScriptNode *pNode = CreateScriptNode(pObj);

            // Add to map
            mScriptNodeMap[pObj->InstanceID()] = pNode;
        }
    }

    // Ensure script nodes have valid positions + build light lists
    for (auto it = mScriptNodeMap.begin(); it != mScriptNodeMap.end(); it++)
    {
        it->second->GeneratePosition();
        it->second->BuildLightList(mpArea);
    }

    u32 NumLightLayers = mpArea->GetLightLayerCount();
    CGraphics::sAreaAmbientColor = CColor::skBlack;

    for (u32 iLyr = 0; iLyr < NumLightLayers; iLyr++)
    {
        u32 NumLights = mpArea->GetLightCount(iLyr);

        for (u32 iLit = 0; iLit < NumLights; iLit++)
        {
            CLight *pLight = mpArea->GetLight(iLyr, iLit);

            if (pLight->GetType() == eLocalAmbient)
                CGraphics::sAreaAmbientColor += pLight->GetColor();

            CreateLightNode(pLight);
        }
    }

    Log::Write( TString::FromInt32(CSceneNode::NumNodes()) + " nodes" );
}

void CScene::SetActiveWorld(CWorld* pWorld)
{
    mpWorld = pWorld;
}

void CScene::ClearScene()
{
    if (mpAreaRootNode)
    {
        mpAreaRootNode->Unparent();
        delete mpAreaRootNode;
    }

    mNodes.clear();
    mNumNodes = 0;

    mpArea = nullptr;
    mpWorld = nullptr;
}

void CScene::AddSceneToRenderer(CRenderer *pRenderer, const SViewInfo& ViewInfo)
{
    // Override show flags in game mode
    FShowFlags ShowFlags = (ViewInfo.GameMode ? gkGameModeShowFlags : ViewInfo.ShowFlags);
    FNodeFlags NodeFlags = NodeFlagsForShowFlags(ShowFlags);

    for (auto it = mNodes.begin(); it != mNodes.end(); it++)
    {
        if (NodeFlags & it->first)
        {
            std::vector<CSceneNode*>& rNodeVec = it->second;

            for (u32 iNode = 0; iNode < rNodeVec.size(); iNode++)
                if (ViewInfo.GameMode || rNodeVec[iNode]->IsVisible())
                    rNodeVec[iNode]->AddToRenderer(pRenderer, ViewInfo);
        }
    }
}

SRayIntersection CScene::SceneRayCast(const CRay& Ray, const SViewInfo& ViewInfo)
{
    FShowFlags ShowFlags = (ViewInfo.GameMode ? gkGameModeShowFlags : ViewInfo.ShowFlags);
    FNodeFlags NodeFlags = NodeFlagsForShowFlags(ShowFlags);
    CRayCollisionTester Tester(Ray);

    for (auto it = mNodes.begin(); it != mNodes.end(); it++)
    {
        if (NodeFlags & it->first)
        {
            std::vector<CSceneNode*>& rNodeVec = it->second;

            for (u32 iNode = 0; iNode < rNodeVec.size(); iNode++)
                if (rNodeVec[iNode]->IsVisible())
                    rNodeVec[iNode]->RayAABoxIntersectTest(Tester, ViewInfo);
        }
    }

    return Tester.TestNodes(ViewInfo);
}

CScriptNode* CScene::ScriptNodeByID(u32 InstanceID)
{
    auto it = mScriptNodeMap.find(InstanceID);

    if (it != mScriptNodeMap.end()) return it->second;
    else return nullptr;
}

CScriptNode* CScene::NodeForObject(CScriptObject *pObj)
{
    return ScriptNodeByID(pObj->InstanceID());
}

CLightNode* CScene::NodeForLight(CLight *pLight)
{
    // Slow. Is there a better way to do this?
    std::vector<CSceneNode*>& rLights = mNodes[eLightNode];

    for (auto it = rLights.begin(); it != rLights.end(); it++)
    {
        CLightNode *pNode = static_cast<CLightNode*>(*it);
        if (pNode->Light() == pLight) return pNode;
    }

    return nullptr;
}

CModel* CScene::GetActiveSkybox()
{
    bool SkyEnabled = false;

    for (u32 iAtt = 0; iAtt < mAreaAttributesObjects.size(); iAtt++)
    {
        const CAreaAttributes& rkAttributes = mAreaAttributesObjects[iAtt];
        if (rkAttributes.IsSkyEnabled()) SkyEnabled = true;

        if (rkAttributes.IsLayerEnabled())
        {
            if (rkAttributes.IsSkyEnabled())
            {
                SkyEnabled = true;
                CModel *pSky = rkAttributes.SkyModel();
                if (pSky) return pSky;
            }
        }
    }

    if (SkyEnabled) return mpWorld->GetDefaultSkybox();
    else return nullptr;
}

CGameArea* CScene::GetActiveArea()
{
    return mpArea;
}

// ************ STATIC ************
FShowFlags CScene::ShowFlagsForNodeFlags(FNodeFlags NodeFlags)
{
    FShowFlags Out;
    if (NodeFlags & eModelNode) Out |= eShowSplitWorld;
    if (NodeFlags & eStaticNode) Out |= eShowMergedWorld;
    if (NodeFlags & eScriptNode) Out |= eShowObjects;
    if (NodeFlags & eCollisionNode) Out |= eShowWorldCollision;
    if (NodeFlags & eLightNode) Out |= eShowLights;
    return Out;
}

FNodeFlags CScene::NodeFlagsForShowFlags(FShowFlags ShowFlags)
{
    FNodeFlags Out = eRootNode;
    if (ShowFlags & eShowSplitWorld) Out |= eModelNode;
    if (ShowFlags & eShowMergedWorld) Out |= eStaticNode;
    if (ShowFlags & eShowWorldCollision) Out |= eCollisionNode;
    if (ShowFlags & eShowObjects) Out |= eScriptNode | eScriptExtraNode;
    if (ShowFlags & eShowLights) Out |= eLightNode;
    return Out;
}
