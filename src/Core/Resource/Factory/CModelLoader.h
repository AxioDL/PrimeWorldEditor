#ifndef CMODELLOADER_H
#define CMODELLOADER_H

#include "CSectionMgrIn.h"
#include "Core/GameProject/CResourceStore.h"
#include "Core/Resource/Model/CBasicModel.h"
#include "Core/Resource/Model/CModel.h"
#include <Common/EGame.h>
#include <Common/FileIO.h>
#include <Common/Flags.h>

#include <assimp/scene.h>

#include <memory>

class CMaterialSet;
class CResourceEntry;
class IInputStream;
enum class EGame;
struct SSurface;

enum class EModelLoaderFlag
{
    None                    = 0x0,
    HalfPrecisionPositions  = 0x1,
    HalfPrecisionNormals    = 0x2,
    LightmapUVs             = 0x4,
    VisibilityGroups        = 0x8,
    Skinned                 = 0x10
};
DECLARE_FLAGS_ENUMCLASS(EModelLoaderFlag, FModelLoaderFlags)

class CModelLoader
{
private:
    TResPtr<CModel> mpModel;
    std::vector<CMaterialSet*> mMaterials;
    CSectionMgrIn *mpSectionMgr = nullptr;
    CAABox mAABox;
    EGame mVersion{};

    uint32 mNumVertices = 0;
    std::vector<CVector3f> mPositions;
    std::vector<CVector3f> mNormals;
    std::vector<CColor> mColors;
    std::vector<CVector2f> mTex0;
    std::vector<CVector2f> mTex1;
    bool mSurfaceUsingTex1 = false;

    uint32 mSurfaceCount = 0;
    std::vector<uint32> mSurfaceOffsets;

    FModelLoaderFlags mFlags{EModelLoaderFlag::None};

    CModelLoader();
    ~CModelLoader();
    void LoadWorldMeshHeader(IInputStream& rModel);
    void LoadAttribArrays(IInputStream& rModel);
    void LoadAttribArraysDKCR(IInputStream& rModel);
    void LoadSurfaceOffsets(IInputStream& rModel);
    SSurface* LoadSurface(IInputStream& rModel);
    void LoadSurfaceHeaderPrime(IInputStream& rModel, SSurface *pSurf);
    void LoadSurfaceHeaderDKCR(IInputStream& rModel, SSurface *pSurf);
    SSurface* LoadAssimpMesh(const aiMesh *pkMesh, CMaterialSet *pSet);

public:
    static std::unique_ptr<CModel> LoadCMDL(IInputStream& rCMDL, CResourceEntry *pEntry);
    static std::unique_ptr<CModel> LoadWorldModel(IInputStream& rMREA, CSectionMgrIn& rBlockMgr, CMaterialSet& rMatSet, EGame Version);
    static std::unique_ptr<CModel> LoadCorruptionWorldModel(IInputStream& rMREA, CSectionMgrIn& rBlockMgr, CMaterialSet& rMatSet, uint32 HeaderSecNum, uint32 GPUSecNum, EGame Version);
    static void BuildWorldMeshes(std::vector<std::unique_ptr<CModel>>& rkIn, std::vector<std::unique_ptr<CModel>>& rOut, bool DeleteInputModels);
    static CModel* ImportAssimpNode(const aiNode *pkNode, const aiScene *pkScene, CMaterialSet& rMatSet);
    static EGame GetFormatVersion(uint32 Version);
};

#endif // CMODELLOADER_H
