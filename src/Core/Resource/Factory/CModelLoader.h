#ifndef CMODELLOADER_H
#define CMODELLOADER_H

#include "CBlockMgrIn.h"
#include "Core/Resource/Model/CBasicModel.h"
#include "Core/Resource/Model/CModel.h"
#include "Core/Resource/CResCache.h"
#include "Core/Resource/EGame.h"
#include <Common/EnumUtil.h>

#include <FileIO/FileIO.h>
#include <assimp/scene.h>

class CModelLoader
{
public:
    enum EModelFlags
    {
        eNoFlags        = 0x0,
        eShortPositions = 0x1,
        eShortNormals   = 0x2,
        eHasTex1        = 0x4,
        eHasVisGroups   = 0x8
    };

private:
    TResPtr<CModel> mpModel;
    std::vector<CMaterialSet*> mMaterials;
    CBlockMgrIn *mpBlockMgr;
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

    EModelFlags mFlags;

    CModelLoader();
    ~CModelLoader();
    void LoadWorldMeshHeader(IInputStream& Model);
    void LoadAttribArrays(IInputStream& Model);
    void LoadAttribArraysDKCR(IInputStream& Model);
    void LoadSurfaceOffsets(IInputStream& Model);
    SSurface* LoadSurface(IInputStream& Model);
    void LoadSurfaceHeaderPrime(IInputStream& Model, SSurface *pSurf);
    void LoadSurfaceHeaderDKCR(IInputStream& Model, SSurface *pSurf);
    SSurface* LoadAssimpMesh(const aiMesh *pMesh, CMaterialSet *pSet);

public:
    static CModel* LoadCMDL(IInputStream& CMDL);
    static CModel* LoadWorldModel(IInputStream& MREA, CBlockMgrIn& BlockMgr, CMaterialSet& MatSet, EGame Version);
    static CModel* LoadCorruptionWorldModel(IInputStream& MREA, CBlockMgrIn& BlockMgr, CMaterialSet& MatSet, u32 HeaderSecNum, u32 GPUSecNum, EGame Version);
    static CModel* ImportAssimpNode(const aiNode *pNode, const aiScene *pScene, CMaterialSet& matSet);
    static EGame GetFormatVersion(u32 Version);
};

DEFINE_ENUM_FLAGS(CModelLoader::EModelFlags)

#endif // CMODELLOADER_H
