#ifndef CMODELCOOKER_H
#define CMODELCOOKER_H

#include "../model/CModel.h"
#include "../EFormatVersion.h"
#include <FileIO/FileIO.h>

class CModelCooker
{
    TResPtr<CModel> mpModel;
    EGame mVersion;
    u32 mNumMatSets;
    u32 mNumSurfaces;
    u32 mNumVertices;
    u8 mVertexFormat;
    std::vector<CVertex> mVertices;
    EVertexDescription mVtxAttribs;

    CModelCooker();
    void GenerateSurfaceData();
    void WriteEditorModel(COutputStream& Out);
    void WriteModelPrime(COutputStream& Out);

public:
    static void WriteCookedModel(CModel *pModel, EGame Version, COutputStream& Out);
    static void WriteUncookedModel(CModel *pModel, COutputStream& Out);
    static u32 GetCMDLVersion(EGame Version);
};

#endif // CMODELCOOKER_H
