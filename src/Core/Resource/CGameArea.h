#ifndef CGAMEAREA_H
#define CGAMEAREA_H

#include "CResource.h"
#include "CCollisionMeshGroup.h"
#include "CLight.h"
#include "CMaterialSet.h"
#include "CPoiToWorld.h"
#include "Core/Resource/Model/CModel.h"
#include "Core/Resource/Model/CStaticModel.h"
#include <Common/types.h>
#include <Math/CQuaternion.h>
#include <Math/CTransform4f.h>

#include <unordered_map>

class CScriptLayer;
class CScriptObject;
class CScriptTemplate;

class CGameArea : public CResource
{
    DECLARE_RESOURCE_TYPE(eArea)
    friend class CAreaLoader;
    friend class CAreaCooker;

    EGame mVersion;
    u32 mWorldIndex;
    u32 mVertexCount;
    u32 mTriangleCount;
    bool mTerrainMerged;
    CTransform4f mTransform;
    CAABox mAABox;

    // Data saved from the original file to help on recook
    std::vector<std::vector<u8>> mSectionDataBuffers;
    u32 mOriginalWorldMeshCount;
    bool mUsesCompression;

    struct SSectionNumber
    {
        CFourCC SectionID;
        u32 Index;
    };
    std::vector<SSectionNumber> mSectionNumbers;

    // Geometry
    CMaterialSet *mMaterialSet;
    std::vector<CModel*> mTerrainModels; // TerrainModels is the original version of each model; this is currently mainly used in the POI map editor
    std::vector<CStaticModel*> mStaticTerrainModels; // StaticTerrainModels is the merged terrain for faster rendering in the world editor
    // Script
    std::vector<CScriptLayer*> mScriptLayers;
    CScriptLayer *mpGeneratorLayer;
    std::unordered_map<u32, CScriptObject*> mObjectMap;
    // Collision
    CCollisionMeshGroup *mpCollision;
    // Lights
    std::vector<std::vector<CLight*>> mLightLayers;
    // Object to Static Geometry Map
    TResPtr<CPoiToWorld> mpPoiToWorldMap;

public:
    CGameArea();
    ~CGameArea();

    void AddWorldModel(CModel *mdl);
    void MergeTerrain();
    void ClearTerrain();
    void ClearScriptLayers();
    u32 TotalInstanceCount() const;
    CScriptObject* InstanceByID(u32 InstanceID);
    CScriptObject* SpawnInstance(CScriptTemplate *pTemplate, CScriptLayer *pLayer,
                                 const CVector3f& rkPosition = CVector3f::skZero,
                                 const CQuaternion& rkRotation = CQuaternion::skIdentity,
                                 const CVector3f& rkScale = CVector3f::skOne,
                                 u32 SuggestedID = -1, u32 SuggestedLayerIndex = -1);
    void DeleteInstance(CScriptObject *pInstance);

    // Inline Accessors
    inline EGame Version() const                                    { return mVersion; }
    inline u32 WorldIndex() const                                   { return mWorldIndex; }
    inline CTransform4f GetTransform() const                        { return mTransform; }
    inline u32 GetTerrainModelCount() const                         { return mTerrainModels.size(); }
    inline u32 GetStaticModelCount() const                          { return mStaticTerrainModels.size(); }
    inline CModel* GetTerrainModel(u32 iMdl) const                  { return mTerrainModels[iMdl]; }
    inline CStaticModel* GetStaticModel(u32 iMdl) const             { return mStaticTerrainModels[iMdl]; }
    inline CCollisionMeshGroup* GetCollision() const                { return mpCollision; }
    inline u32 GetScriptLayerCount() const                          { return mScriptLayers.size(); }
    inline CScriptLayer* GetScriptLayer(u32 Index) const            { return mScriptLayers[Index]; }
    inline CScriptLayer* GetGeneratorLayer() const                  { return mpGeneratorLayer; }
    inline u32 GetLightLayerCount() const                           { return mLightLayers.size(); }
    inline u32 GetLightCount(u32 LayerIndex) const                  { return (LayerIndex < mLightLayers.size() ? mLightLayers[LayerIndex].size() : 0); }
    inline CLight* GetLight(u32 LayerIndex, u32 LightIndex) const   { return mLightLayers[LayerIndex][LightIndex]; }
    inline CPoiToWorld* GetPoiToWorldMap() const                    { return mpPoiToWorldMap; }
    inline CAABox AABox() const                                     { return mAABox; }

    inline void SetWorldIndex(u32 NewWorldIndex)                    { mWorldIndex = NewWorldIndex; }
};

#endif // CGAMEAREA_H
