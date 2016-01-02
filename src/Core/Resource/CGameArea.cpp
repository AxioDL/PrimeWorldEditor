#include "CGameArea.h"
#include "Core/Resource/Script/CScriptLayer.h"
#include "Core/Render/CRenderer.h"

CGameArea::CGameArea() : CResource()
{
    mVertexCount = 0;
    mTriangleCount = 0;
    mTerrainMerged = false;
    mMaterialSet = nullptr;
    mpGeneratorLayer = nullptr;
    mCollision = nullptr;
}

CGameArea::~CGameArea()
{
    ClearTerrain();

    delete mCollision;
    delete mpGeneratorLayer;

    for (u32 iSCLY = 0; iSCLY < mScriptLayers.size(); iSCLY++)
        delete mScriptLayers[iSCLY];

    for (u32 lyr = 0; lyr < mLightLayers.size(); lyr++)
        for (u32 lit = 0; lit < mLightLayers[lyr].size(); lit++)
            delete mLightLayers[lyr][lit];
}

void CGameArea::AddWorldModel(CModel *mdl)
{
    mTerrainModels.push_back(mdl);
    mVertexCount += mdl->GetVertexCount();
    mTriangleCount += mdl->GetTriangleCount();
    mAABox.ExpandBounds(mdl->AABox());
}

void CGameArea::MergeTerrain()
{
    if (mTerrainMerged) return;

    // Nothing really complicated here - iterate through every terrain submesh, add each to a static model
    for (u32 iMdl = 0; iMdl < mTerrainModels.size(); iMdl++)
    {
        CModel *pMdl = mTerrainModels[iMdl];
        u32 SubmeshCount = pMdl->GetSurfaceCount();

        for (u32 iSurf = 0; iSurf < SubmeshCount; iSurf++)
        {
            SSurface *pSurf = pMdl->GetSurface(iSurf);
            CMaterial *pMat = mMaterialSet->MaterialByIndex(pSurf->MaterialID);

            bool newMat = true;
            for (std::vector<CStaticModel*>::iterator it = mStaticTerrainModels.begin(); it != mStaticTerrainModels.end(); it++)
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
                    mStaticTerrainModels.erase(it);
                    mStaticTerrainModels.push_back(pStatic);
                    newMat = false;
                    break;
                }
            }

            if (newMat)
            {
                CStaticModel *pStatic = new CStaticModel(pMat);
                pStatic->AddSurface(pSurf);
                mStaticTerrainModels.push_back(pStatic);
            }
        }
    }
}

void CGameArea::ClearTerrain()
{
    for (u32 t = 0; t < mTerrainModels.size(); t++)
        delete mTerrainModels[t];
    mTerrainModels.clear();

    for (u32 s = 0; s < mStaticTerrainModels.size(); s++)
        delete mStaticTerrainModels[s];
    mStaticTerrainModels.clear();

    if (mMaterialSet) delete mMaterialSet;

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
    delete mpGeneratorLayer;
    mpGeneratorLayer = nullptr;
}

EGame CGameArea::Version()
{
    return mVersion;
}

CTransform4f CGameArea::GetTransform()
{
    return mTransform;
}

u32 CGameArea::GetTerrainModelCount()
{
    return mTerrainModels.size();
}

u32 CGameArea::GetStaticModelCount()
{
    return mStaticTerrainModels.size();
}

CModel* CGameArea::GetTerrainModel(u32 mdl)
{
    return mTerrainModels[mdl];
}

CStaticModel* CGameArea::GetStaticModel(u32 mdl)
{
    return mStaticTerrainModels[mdl];
}

CCollisionMeshGroup* CGameArea::GetCollision()
{
    return mCollision;
}

u32 CGameArea::GetScriptLayerCount()
{
    return mScriptLayers.size();
}

CScriptLayer* CGameArea::GetScriptLayer(u32 index)
{
    return mScriptLayers[index];
}

CScriptLayer* CGameArea::GetGeneratorLayer()
{
    return mpGeneratorLayer;
}

CScriptObject* CGameArea::GetInstanceByID(u32 InstanceID)
{
    auto it = mObjectMap.find(InstanceID);
    if (it != mObjectMap.end()) return it->second;
    else return nullptr;
}

u32 CGameArea::GetLightLayerCount()
{
    return mLightLayers.size();
}

u32 CGameArea::GetLightCount(u32 layer)
{
    if (mLightLayers.empty()) return 0;
    return mLightLayers[layer].size();
}

CLight* CGameArea::GetLight(u32 layer, u32 light)
{
    return mLightLayers[layer][light];
}

CAABox CGameArea::AABox()
{
    return mAABox;
}
