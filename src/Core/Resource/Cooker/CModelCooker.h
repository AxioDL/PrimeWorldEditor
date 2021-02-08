#ifndef CMODELCOOKER_H
#define CMODELCOOKER_H

#include "Core/Resource/Model/CModel.h"
#include <Common/EGame.h>
#include <Common/FileIO.h>

class CModelCooker
{
    TResPtr<CModel> mpModel;
    EGame mVersion{};
    uint32 mNumMatSets = 0;
    uint32 mNumSurfaces = 0;
    uint32 mNumVertices = 0;
    uint8 mVertexFormat = 0;
    std::vector<CVertex> mVertices;
    FVertexDescription mVtxAttribs{};

    CModelCooker();
    void GenerateSurfaceData();
    void WriteEditorModel(IOutputStream& rOut);
    void WriteModelPrime(IOutputStream& rOut);

public:
    static bool CookCMDL(CModel *pModel, IOutputStream& rOut);
    static uint32 GetCMDLVersion(EGame Version);
};

#endif // CMODELCOOKER_H
