#ifndef CMODELCOOKER_H
#define CMODELCOOKER_H

#include "Core/Resource/Model/CModel.h"
#include <FileIO/FileIO.h>
#include <Common/EGame.h>

class CModelCooker
{
    TResPtr<CModel> mpModel;
    EGame mVersion;
    u32 mNumMatSets;
    u32 mNumSurfaces;
    u32 mNumVertices;
    u8 mVertexFormat;
    std::vector<CVertex> mVertices;
    FVertexDescription mVtxAttribs;

    CModelCooker();
    void GenerateSurfaceData();
    void WriteEditorModel(IOutputStream& rOut);
    void WriteModelPrime(IOutputStream& rOut);

public:
    static bool CookCMDL(CModel *pModel, IOutputStream& rOut);
    static u32 GetCMDLVersion(EGame Version);
};

#endif // CMODELCOOKER_H
