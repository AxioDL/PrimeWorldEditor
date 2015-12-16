#ifndef CMODELCOOKER_H
#define CMODELCOOKER_H

#include "Core/Resource/Model/CModel.h"
#include "Core/Resource/EFormatVersion.h"
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
    void WriteEditorModel(IOutputStream& Out);
    void WriteModelPrime(IOutputStream& Out);

public:
    static void WriteCookedModel(CModel *pModel, EGame Version, IOutputStream& Out);
    static void WriteUncookedModel(CModel *pModel, IOutputStream& Out);
    static u32 GetCMDLVersion(EGame Version);
};

#endif // CMODELCOOKER_H
