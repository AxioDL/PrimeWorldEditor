#include "CGameArea.h"
#include "Core/Resource/Script/CScriptLayer.h"
#include "Core/Render/CRenderer.h"

CGameArea::CGameArea(CResourceEntry *pEntry)
    : CResource(pEntry)
{
}

CGameArea::~CGameArea()
{
    ClearTerrain();
}

std::unique_ptr<CDependencyTree> CGameArea::BuildDependencyTree() const
{
    // Base dependencies
    auto pTree = std::make_unique<CAreaDependencyTree>();

    std::set<CAssetID> MatTextures;
    mpMaterialSet->GetUsedTextureIDs(MatTextures);

    for (const auto& id : MatTextures)
        pTree->AddDependency(id);

    pTree->AddDependency(mPathID);

    if (Game() >= EGame::EchoesDemo)
    {
        pTree->AddDependency(mPortalAreaID);
        pTree->AddDependency(mpPoiToWorldMap);
    }
    
    // Extra deps
    for (const auto& dep : mExtraAreaDeps)
        pTree->AddDependency(dep);

    // Layer dependencies
    std::vector<CAssetID> DummyDeps;

    for (uint32 iLayer = 0; iLayer < mScriptLayers.size(); iLayer++)
    {
        const std::vector<CAssetID>& rkExtras = (mExtraLayerDeps.size() > iLayer ? mExtraLayerDeps[iLayer] : DummyDeps);
        pTree->AddScriptLayer(mScriptLayers[iLayer].get(), rkExtras);
    }

    return pTree;
}

void CGameArea::AddWorldModel(std::unique_ptr<CModel>&& pModel)
{
    mVertexCount += pModel->GetVertexCount();
    mTriangleCount += pModel->GetTriangleCount();
    mAABox.ExpandBounds(pModel->AABox());

    mWorldModels.push_back(std::move(pModel));
}

void CGameArea::MergeTerrain()
{
    if (mTerrainMerged) return;

    // Nothing really complicated here - iterate through every terrain submesh, add each to a static model
    for (auto& pMdl : mWorldModels)
    {
        const size_t SubmeshCount = pMdl->GetSurfaceCount();

        for (size_t iSurf = 0; iSurf < SubmeshCount; iSurf++)
        {
            SSurface *pSurf = pMdl->GetSurface(iSurf);
            CMaterial *pMat = mpMaterialSet->MaterialByIndex(pSurf->MaterialID, false);

            bool NewMat = true;
            for (auto it = mStaticWorldModels.begin(); it != mStaticWorldModels.end(); ++it)
            {
                if ((*it)->GetMaterial() == pMat)
                {
                    // When we append a new submesh to an existing static model, we bump it to the back of the vector.
                    // This is because mesh ordering actually matters sometimes
                    // (particularly with multi-layered transparent meshes)
                    // so we need to at least try to maintain it.
                    // This is maybe not the most efficient way to do this, but it works.
                    auto pStatic = std::move(*it);
                    pStatic->AddSurface(pSurf);
                    mStaticWorldModels.erase(it);
                    mStaticWorldModels.push_back(std::move(pStatic));
                    NewMat = false;
                    break;
                }
            }

            if (NewMat)
            {
                auto pStatic = std::make_unique<CStaticModel>(pMat);
                pStatic->AddSurface(pSurf);
                mStaticWorldModels.push_back(std::move(pStatic));
            }
        }
    }
}

void CGameArea::ClearTerrain()
{
    mWorldModels.clear();
    mStaticWorldModels.clear();

    if (mpMaterialSet)
        delete mpMaterialSet;

    mVertexCount = 0;
    mTriangleCount = 0;
    mTerrainMerged = false;
    mAABox = CAABox::Infinite();
}

void CGameArea::ClearScriptLayers()
{
    mScriptLayers.clear();
}

size_t CGameArea::TotalInstanceCount() const
{
    size_t Num = 0;

    for (const auto& layer : mScriptLayers)
        Num += layer->NumInstances();

    return Num;
}

CScriptObject* CGameArea::InstanceByID(uint32 InstanceID)
{
    const auto it = mObjectMap.find(InstanceID);

    if (it != mObjectMap.cend())
        return it->second;

    return nullptr;
}

uint32 CGameArea::FindUnusedInstanceID() const
{
    uint32 InstanceID = (mWorldIndex << 16) | 1;

    while (true)
    {
        auto it = mObjectMap.find(InstanceID);

        if (it == mObjectMap.end())
            break;
        else
            InstanceID++;
    }

    return InstanceID;
}

CScriptObject* CGameArea::SpawnInstance(CScriptTemplate *pTemplate,
                                        CScriptLayer *pLayer,
                                        const CVector3f& rkPosition /*= CVector3f::skZero*/,
                                        const CQuaternion& rkRotation /*= CQuaternion::skIdentity*/,
                                        const CVector3f& rkScale /*= CVector3f::skOne*/,
                                        uint32 SuggestedID /*= -1*/,
                                        uint32 SuggestedLayerIndex /*= -1*/ )
{
    // Verify we can fit another instance in this area.
    const size_t NumInstances = TotalInstanceCount();

    if (NumInstances >= 0xFFFF)
    {
        errorf("Unable to spawn a new script instance; too many instances in area (%zu)", NumInstances);
        return nullptr;
    }

    // Check whether the suggested instance ID is valid
    uint32 InstanceID = SuggestedID;

    if (InstanceID != UINT32_MAX)
    {
        if (mObjectMap.find(InstanceID) == mObjectMap.cend())
            InstanceID = UINT32_MAX;
    }

    // If not valid (or if there's no suggested ID) then determine a new instance ID
    if (InstanceID == UINT32_MAX)
    {
        // Determine layer index
        const uint32 LayerIndex = pLayer->AreaIndex();

        if (LayerIndex == UINT32_MAX)
        {
            errorf("Unable to spawn a new script instance; invalid script layer passed in");
            return nullptr;
        }

        // Look for a valid instance ID
        InstanceID = FindUnusedInstanceID();
    }

    // Spawn instance
    auto* pInstance = new CScriptObject(InstanceID, this, pLayer, pTemplate);
    pInstance->EvaluateProperties();
    pInstance->SetPosition(rkPosition);
    pInstance->SetRotation(rkRotation.ToEuler());
    pInstance->SetScale(rkScale);
    pInstance->SetName(pTemplate->Name());
    if (pTemplate->Game() < EGame::EchoesDemo)
        pInstance->SetActive(true);
    pLayer->AddInstance(pInstance, SuggestedLayerIndex);
    mObjectMap[InstanceID] = pInstance;
    return pInstance;
}

void CGameArea::AddInstanceToArea(CScriptObject *pInstance)
{
    // Used for undo after deleting an instance.
    // In the future the script loader should go through SpawnInstance to avoid the need for this function.
    mObjectMap[pInstance->InstanceID()] = pInstance;
}

void CGameArea::DeleteInstance(CScriptObject *pInstance)
{
    pInstance->BreakAllLinks();
    pInstance->Layer()->RemoveInstance(pInstance);
    pInstance->Template()->RemoveObject(pInstance);

    auto it = mObjectMap.find(pInstance->InstanceID());
    if (it != mObjectMap.end()) mObjectMap.erase(it);

    if (mpPoiToWorldMap && mpPoiToWorldMap->HasPoiMappings(pInstance->InstanceID()))
        mpPoiToWorldMap->RemovePoi(pInstance->InstanceID());

    delete pInstance;
}

void CGameArea::ClearExtraDependencies()
{
    if (mExtraAreaDeps.empty() || !mExtraLayerDeps.empty())
    {
        mExtraAreaDeps.clear();
        mExtraLayerDeps.clear();
        Entry()->UpdateDependencies();
    }
}
