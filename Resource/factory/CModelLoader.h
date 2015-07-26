#ifndef CMODELLOADER_H
#define CMODELLOADER_H

#include "../model/SModelData.h"
#include "../model/CBasicModel.h"
#include "../model/CModel.h"
#include "../EFormatVersion.h"
#include "CBlockMgrIn.h"
#include <FileIO/FileIO.h>
#include <Core/CResCache.h>
#include <Common/EnumUtil.h>

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
    CModel *mpModel;
    std::vector<CMaterialSet*> mMaterials;
    CBlockMgrIn *mpBlockMgr;
    CAABox mAABox;
    EGame mVersion;

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
    void LoadWorldMeshHeader(CInputStream& Model);
    void LoadAttribArrays(CInputStream& Model);
    void LoadAttribArraysDKCR(CInputStream& Model);
    void LoadSurfaceOffsets(CInputStream& Model);
    SModelData* LoadSurfaces(CInputStream& Model);
    void LoadSurfaceHeaderPrime(CInputStream& Model, SSurface *pSurf);
    void LoadSurfaceHeaderDKCR(CInputStream& Model, SSurface *pSurf);

public:
    static CModel* LoadCMDL(CInputStream& CMDL);
    static SModelData* LoadWorldModel(CInputStream& MREA, CBlockMgrIn& BlockMgr, CMaterialSet& MatSet, EGame Version);
    static SModelData* LoadCorruptionWorldModel(CInputStream& MREA, CBlockMgrIn& BlockMgr, CMaterialSet& MatSet, u32 HeaderSecNum, u32 GPUSecNum, EGame Version);
    static EGame GetFormatVersion(u32 Version);
};

DEFINE_ENUM_FLAGS(CModelLoader::EModelFlags)

#endif // CMODELLOADER_H
