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

#include <unordered_map>

class CScriptLayer;
class CScriptObject;
class CScriptTemplate;

class CGameArea : public CResource
{
    DECLARE_RESOURCE_TYPE(Area)
    friend class CAreaLoader;
    friend class CAreaCooker;

    uint32 mWorldIndex;
    uint32 mVertexCount;
    uint32 mTriangleCount;
    bool mTerrainMerged;
    CTransform4f mTransform;
    CAABox mAABox;

    // Data saved from the original file to help on recook
    std::vector<std::vector<uint8>> mSectionDataBuffers;
    uint32 mOriginalWorldMeshCount;
    bool mUsesCompression;

    struct SSectionNumber
    {
        CFourCC SectionID;
        uint32 Index;
    };
    std::vector<SSectionNumber> mSectionNumbers;

    // Geometry
    CMaterialSet *mpMaterialSet;
    std::vector<CModel*> mWorldModels; // TerrainModels is the original version of each model; this is currently mainly used in the POI map editor
    std::vector<CStaticModel*> mStaticWorldModels; // StaticTerrainModels is the merged terrain for faster rendering in the world editor
    // Script
    std::vector<CScriptLayer*> mScriptLayers;
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
    CGameArea(CResourceEntry *pEntry = nullptr);
    ~CGameArea();
    CDependencyTree* BuildDependencyTree() const override;

    void AddWorldModel(CModel *pModel);
    void MergeTerrain();
    void ClearTerrain();
    void ClearScriptLayers();
    uint32 TotalInstanceCount() const;
    CScriptObject* InstanceByID(uint32 InstanceID);
    uint32 FindUnusedInstanceID() const;
    CScriptObject* SpawnInstance(CScriptTemplate *pTemplate, CScriptLayer *pLayer,
                                 const CVector3f& rkPosition = CVector3f::skZero,
                                 const CQuaternion& rkRotation = CQuaternion::skIdentity,
                                 const CVector3f& rkScale = CVector3f::skOne,
                                 uint32 SuggestedID = -1, uint32 SuggestedLayerIndex = -1);
    void AddInstanceToArea(CScriptObject *pInstance);
    void DeleteInstance(CScriptObject *pInstance);
    void ClearExtraDependencies();

    // Accessors
    uint32 WorldIndex() const                                    { return mWorldIndex; }
    CTransform4f Transform() const                               { return mTransform; }
    CMaterialSet* Materials() const                              { return mpMaterialSet; }
    uint32 NumWorldModels() const                                { return mWorldModels.size(); }
    uint32 NumStaticModels() const                               { return mStaticWorldModels.size(); }
    CModel* TerrainModel(uint32 iMdl) const                      { return mWorldModels[iMdl]; }
    CStaticModel* StaticModel(uint32 iMdl) const                 { return mStaticWorldModels[iMdl]; }
    CCollisionMeshGroup* Collision() const                       { return mpCollision.get(); }
    uint32 NumScriptLayers() const                               { return mScriptLayers.size(); }
    CScriptLayer* ScriptLayer(uint32 Index) const                { return mScriptLayers[Index]; }
    uint32 NumLightLayers() const                                { return mLightLayers.size(); }
    uint32 NumLights(uint32 LayerIndex) const                    { return (LayerIndex < mLightLayers.size() ? mLightLayers[LayerIndex].size() : 0); }
    CLight* Light(uint32 LayerIndex, uint32 LightIndex)          { return &mLightLayers[LayerIndex][LightIndex]; }
    CAssetID PathID() const                                      { return mPathID; }
    CPoiToWorld* PoiToWorldMap() const                           { return mpPoiToWorldMap; }
    CAssetID PortalAreaID() const                                { return mPortalAreaID; }
    CAABox AABox() const                                         { return mAABox; }

    void SetWorldIndex(uint32 NewWorldIndex)                     { mWorldIndex = NewWorldIndex; }
};

#endif // CGAMEAREA_H
