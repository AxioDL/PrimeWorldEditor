#include "CGameArea.h"
#include "Core/Resource/Script/CScriptLayer.h"
#include "Core/Render/CRenderer.h"

CGameArea::CGameArea(CResourceEntry *pEntry /*= 0*/)
    : CResource(pEntry)
    , mWorldIndex(-1)
    , mVertexCount(0)
    , mTriangleCount(0)
    , mTerrainMerged(false)
    , mOriginalWorldMeshCount(0)
    , mUsesCompression(false)
    , mpMaterialSet(nullptr)
    , mpCollision(nullptr)
{
}

CGameArea::~CGameArea()
{
    ClearTerrain();

    for (uint32 iSCLY = 0; iSCLY < mScriptLayers.size(); iSCLY++)
        delete mScriptLayers[iSCLY];

    for (uint32 iLyr = 0; iLyr < mLightLayers.size(); iLyr++)
        for (uint32 iLight = 0; iLight < mLightLayers[iLyr].size(); iLight++)
            delete mLightLayers[iLyr][iLight];
}

CDependencyTree* CGameArea::BuildDependencyTree() const
{
    // Base dependencies
    CAreaDependencyTree *pTree = new CAreaDependencyTree();

    std::set<CAssetID> MatTextures;
    mpMaterialSet->GetUsedTextureIDs(MatTextures);

    for (auto Iter = MatTextures.begin(); Iter != MatTextures.end(); Iter++)
        pTree->AddDependency(*Iter);

    pTree->AddDependency(mPathID);

    if (Game() >= EGame::EchoesDemo)
    {
        pTree->AddDependency(mPortalAreaID);
        pTree->AddDependency(mpPoiToWorldMap);
    }
    
    // Extra deps
    for (uint32 iDep = 0; iDep < mExtraAreaDeps.size(); iDep++)
        pTree->AddDependency(mExtraAreaDeps[iDep]);

    // Layer dependencies
    std::vector<CAssetID> DummyDeps;

    for (uint32 iLayer = 0; iLayer < mScriptLayers.size(); iLayer++)
    {
        const std::vector<CAssetID>& rkExtras = (mExtraLayerDeps.size() > iLayer ? mExtraLayerDeps[iLayer] : DummyDeps);
        pTree->AddScriptLayer(mScriptLayers[iLayer], rkExtras);
    }

    return pTree;
}

void CGameArea::AddWorldModel(CModel *pModel)
{
    mWorldModels.push_back(pModel);
    mVertexCount += pModel->GetVertexCount();
    mTriangleCount += pModel->GetTriangleCount();
    mAABox.ExpandBounds(pModel->AABox());
}

void CGameArea::MergeTerrain()
{
    if (mTerrainMerged) return;

    // Nothing really complicated here - iterate through every terrain submesh, add each to a static model
    for (uint32 iMdl = 0; iMdl < mWorldModels.size(); iMdl++)
    {
        CModel *pMdl = mWorldModels[iMdl];
        uint32 SubmeshCount = pMdl->GetSurfaceCount();

        for (uint32 iSurf = 0; iSurf < SubmeshCount; iSurf++)
        {
            SSurface *pSurf = pMdl->GetSurface(iSurf);
            CMaterial *pMat = mpMaterialSet->MaterialByIndex(pSurf->MaterialID);

            bool NewMat = true;
            for (std::vector<CStaticModel*>::iterator it = mStaticWorldModels.begin(); it != mStaticWorldModels.end(); it++)
            {
                if ((*it)->GetMaterial() == pMat)
                {
                    // When we append a new submesh to an existing static model, we bump it to the back of the vector.
                    // This is because mesh ordering actually matters sometimes
                    // (particularly with multi-layered transparent meshes)
                    // so we need to at least try to maintain it.
                    // This is maybe not the most efficient way to do this, but it works.
                    CStaticModel *pStatic = *it;
                    pStatic->AddSurface(pSurf);
                    mStaticWorldModels.erase(it);
                    mStaticWorldModels.push_back(pStatic);
                    NewMat = false;
                    break;
                }
            }

            if (NewMat)
            {
                CStaticModel *pStatic = new CStaticModel(pMat);
                pStatic->AddSurface(pSurf);
                mStaticWorldModels.push_back(pStatic);
            }
        }
    }
}

void CGameArea::ClearTerrain()
{
    for (uint32 iModel = 0; iModel < mWorldModels.size(); iModel++)
        delete mWorldModels[iModel];
    mWorldModels.clear();

    for (uint32 iStatic = 0; iStatic < mStaticWorldModels.size(); iStatic++)
        delete mStaticWorldModels[iStatic];
    mStaticWorldModels.clear();

    if (mpMaterialSet) delete mpMaterialSet;

    mVertexCount = 0;
    mTriangleCount = 0;
    mTerrainMerged = false;
    mAABox = CAABox::skInfinite;
}

void CGameArea::ClearScriptLayers()
{
    for (auto it = mScriptLayers.begin(); it != mScriptLayers.end(); it++)
        delete *it;
    mScriptLayers.clear();
}

uint32 CGameArea::TotalInstanceCount() const
{
    uint32 Num = 0;

    for (uint32 iLyr = 0; iLyr < mScriptLayers.size(); iLyr++)
        Num += mScriptLayers[iLyr]->NumInstances();

    return Num;
}

CScriptObject* CGameArea::InstanceByID(uint32 InstanceID)
{
    auto it = mObjectMap.find(InstanceID);
    if (it != mObjectMap.end()) return it->second;
    else return nullptr;
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
    uint32 NumInstances = TotalInstanceCount();

    if (NumInstances >= 0xFFFF)
    {
        errorf("Unable to spawn a new script instance; too many instances in area (%d)", NumInstances);
        return nullptr;
    }

    // Check whether the suggested instance ID is valid
    uint32 InstanceID = SuggestedID;

    if (InstanceID != -1)
    {
        if (mObjectMap.find(InstanceID) == mObjectMap.end())
            InstanceID = -1;
    }

    // If not valid (or if there's no suggested ID) then determine a new instance ID
    if (InstanceID == -1)
    {
        // Determine layer index
        uint32 LayerIndex = pLayer->AreaIndex();

        if (LayerIndex == -1)
        {
            errorf("Unable to spawn a new script instance; invalid script layer passed in");
            return nullptr;
        }

        // Look for a valid instance ID
        InstanceID = FindUnusedInstanceID();
    }

    // Spawn instance
    CScriptObject *pInstance = new CScriptObject(InstanceID, this, pLayer, pTemplate);
    pInstance->EvaluateProperties();
    pInstance->SetPosition(rkPosition);
    pInstance->SetRotation(rkRotation.ToEuler());
    pInstance->SetScale(rkScale);
    pInstance->SetName(pTemplate->Name());
    if (pTemplate->Game() < EGame::EchoesDemo) pInstance->SetActive(true);
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
