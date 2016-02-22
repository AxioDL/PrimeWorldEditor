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
#include <Math/CTransform4f.h>

#include <unordered_map>

class CScriptLayer;
class CScriptObject;

class CGameArea : public CResource
{
    DECLARE_RESOURCE_TYPE(eArea)
    friend class CAreaLoader;
    friend class CAreaCooker;

    EGame mVersion;
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
    CCollisionMeshGroup *mCollision;
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

    // Getters
    EGame Version();
    CTransform4f GetTransform();
    u32 GetTerrainModelCount();
    u32 GetStaticModelCount();
    CModel* GetTerrainModel(u32 mdl);
    CStaticModel* GetStaticModel(u32 mdl);
    CCollisionMeshGroup* GetCollision();
    u32 GetScriptLayerCount();
    CScriptLayer* GetScriptLayer(u32 index);
    CScriptLayer* GetGeneratorLayer();
    CScriptObject* GetInstanceByID(u32 InstanceID);
    u32 GetLightLayerCount();
    u32 GetLightCount(u32 layer);
    CLight* GetLight(u32 layer, u32 light);
    CPoiToWorld* GetPoiToWorldMap();
    CAABox AABox();
};

#endif // CGAMEAREA_H
