#ifndef CGAMEAREA_H
#define CGAMEAREA_H

#include "Core/Resource/CResource.h"
#include "Core/Resource/CLight.h"
#include "Core/Resource/CMaterialSet.h"
#include "Core/Resource/CPoiToWorld.h"
#include "Core/Resource/Collision/CCollisionMeshGroup.h"
#include "Core/Resource/Model/CModel.h"
#include "Core/Resource/Model/CStaticModel.h"
#include <Common/BasicTypes.h>
#include <Common/Math/CQuaternion.h>
#include <Common/Math/CTransform4f.h>

#include <memory>
#include <unordered_map>
#include <vector>

class CScriptLayer;
class CScriptObject;
class CScriptTemplate;

class CGameArea : public CResource
{
    DECLARE_RESOURCE_TYPE(Area)
    friend class CAreaLoader;
    friend class CAreaCooker;

    uint32 mWorldIndex = UINT32_MAX;
    uint32 mVertexCount = 0;
    uint32 mTriangleCount = 0;
    bool mTerrainMerged = false;
    CTransform4f mTransform;
    CAABox mAABox;

    // Data saved from the original file to help on recook
    std::vector<std::vector<uint8>> mSectionDataBuffers;
    uint32 mOriginalWorldMeshCount = 0;
    bool mUsesCompression = false;

    struct SSectionNumber
    {
        CFourCC SectionID;
        uint32 Index;
    };
    std::vector<SSectionNumber> mSectionNumbers;

    // Geometry
    CMaterialSet *mpMaterialSet = nullptr;
    std::vector<std::unique_ptr<CModel>> mWorldModels; // TerrainModels is the original version of each model; this is currently mainly used in the POI map editor
    std::vector<std::unique_ptr<CStaticModel>> mStaticWorldModels; // StaticTerrainModels is the merged terrain for faster rendering in the world editor
    // Script
    std::vector<std::unique_ptr<CScriptLayer>> mScriptLayers;
    std::unordered_map<uint32, CScriptObject*> mObjectMap;
    // Collision
    std::unique_ptr<CCollisionMeshGroup> mpCollision;
    // Lights
    std::vector<std::vector<CLight>> mLightLayers;
    // Path Mesh
    CAssetID mPathID;
    // Portal Area
    CAssetID mPortalAreaID;
    // Object to Static Geometry Map
    TResPtr<CPoiToWorld> mpPoiToWorldMap;
    // Dependencies
    std::vector<CAssetID> mExtraAreaDeps;
    std::vector< std::vector<CAssetID> > mExtraLayerDeps;

public:
    explicit CGameArea(CResourceEntry *pEntry = nullptr);
    ~CGameArea() override;
    std::unique_ptr<CDependencyTree> BuildDependencyTree() const override;

    void AddWorldModel(std::unique_ptr<CModel>&& pModel);
    void MergeTerrain();
    void ClearTerrain();
    void ClearScriptLayers();
    size_t TotalInstanceCount() const;
    CScriptObject* InstanceByID(uint32 InstanceID);
    uint32 FindUnusedInstanceID() const;
    CScriptObject* SpawnInstance(CScriptTemplate* pTemplate, CScriptLayer* pLayer,
                                 const CVector3f& rkPosition = CVector3f::Zero(),
                                 const CQuaternion& rkRotation = CQuaternion::Identity(),
                                 const CVector3f& rkScale = CVector3f::One(),
                                 uint32 SuggestedID = UINT32_MAX,
                                 uint32 SuggestedLayerIndex = UINT32_MAX);
    void AddInstanceToArea(CScriptObject *pInstance);
    void DeleteInstance(CScriptObject *pInstance);
    void ClearExtraDependencies();

    // Accessors
    uint32 WorldIndex() const                                    { return mWorldIndex; }
    CTransform4f Transform() const                               { return mTransform; }
    CMaterialSet* Materials() const                              { return mpMaterialSet; }
    size_t NumWorldModels() const                                { return mWorldModels.size(); }
    size_t NumStaticModels() const                               { return mStaticWorldModels.size(); }
    CModel* TerrainModel(size_t iMdl) const                      { return mWorldModels[iMdl].get(); }
    CStaticModel* StaticModel(size_t iMdl) const                 { return mStaticWorldModels[iMdl].get(); }
    CCollisionMeshGroup* Collision() const                       { return mpCollision.get(); }
    size_t NumScriptLayers() const                               { return mScriptLayers.size(); }
    CScriptLayer* ScriptLayer(size_t Index) const                { return mScriptLayers[Index].get(); }
    size_t NumLightLayers() const                                { return mLightLayers.size(); }
    size_t NumLights(size_t LayerIndex) const                    { return (LayerIndex < mLightLayers.size() ? mLightLayers[LayerIndex].size() : 0); }
    CLight* Light(size_t LayerIndex, size_t LightIndex)          { return &mLightLayers[LayerIndex][LightIndex]; }
    CAssetID PathID() const                                      { return mPathID; }
    CPoiToWorld* PoiToWorldMap() const                           { return mpPoiToWorldMap; }
    CAssetID PortalAreaID() const                                { return mPortalAreaID; }
    CAABox AABox() const                                         { return mAABox; }

    void SetWorldIndex(uint32 NewWorldIndex)                     { mWorldIndex = NewWorldIndex; }
};

#endif // CGAMEAREA_H
