#ifndef CMODELLOADER_H
#define CMODELLOADER_H

#include "CSectionMgrIn.h"
#include "Core/Resource/Model/CBasicModel.h"
#include "Core/Resource/Model/CModel.h"
#include "Core/Resource/CResCache.h"
#include "Core/Resource/EGame.h"
#include <Common/Flags.h>

#include <FileIO/FileIO.h>
#include <assimp/scene.h>

class CModelLoader
{
public:
    enum EModelFlag
    {
        eNoFlags        = 0x0,
        eShortPositions = 0x1,
        eShortNormals   = 0x2,
        eHasTex1        = 0x4,
        eHasVisGroups   = 0x8
    };
    DECLARE_FLAGS(EModelFlag, FModelFlags)

private:
    TResPtr<CModel> mpModel;
    std::vector<CMaterialSet*> mMaterials;
    CSectionMgrIn *mpSectionMgr;
    CAABox mAABox;
    EGame mVersion;

    u32 mNumVertices;
    std::vector<CVector3f> mPositions;
    std::vector<CVector3f> mNormals;
    std::vector<CColor> mColors;
    std::vector<CVector2f> mTex0;
    std::vector<CVector2f> mTex1;
    bool mSurfaceUsingTex1;

    u32 mSurfaceCount;
    std::vector<u32> mSurfaceOffsets;

    FModelFlags mFlags;

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
    static CModel* LoadCMDL(IInputStream& rCMDL);
    static CModel* LoadWorldModel(IInputStream& rMREA, CSectionMgrIn& rBlockMgr, CMaterialSet& rMatSet, EGame Version);
    static CModel* LoadCorruptionWorldModel(IInputStream& rMREA, CSectionMgrIn& rBlockMgr, CMaterialSet& rMatSet, u32 HeaderSecNum, u32 GPUSecNum, EGame Version);
    static void BuildWorldMeshes(const std::vector<CModel*>& rkIn, std::vector<CModel*>& rOut, bool DeleteInputModels);
    static CModel* ImportAssimpNode(const aiNode *pkNode, const aiScene *pkScene, CMaterialSet& rMatSet);
    static EGame GetFormatVersion(u32 Version);
};

#endif // CMODELLOADER_H
