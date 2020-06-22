#ifndef CMATERIALCOOKER_H
#define CMATERIALCOOKER_H

#include "Core/Resource/CMaterial.h"
#include "Core/Resource/CMaterialSet.h"
#include <Common/EGame.h>

class CMaterialCooker
{
    CMaterialSet *mpSet = nullptr;
    CMaterial *mpMat = nullptr;
    EGame mVersion{};
    std::vector<uint32> mTextureIDs;
    std::vector<uint64> mMaterialHashes;

    CMaterialCooker();
    uint32 ConvertFromVertexDescription(FVertexDescription VtxDesc);
    void WriteMatSetPrime(IOutputStream& rOut);
    void WriteMatSetCorruption(IOutputStream& rOut);
    void WriteMaterialPrime(IOutputStream& rOut);
    void WriteMaterialCorruption(IOutputStream& rOut);

public:
    static void WriteCookedMatSet(CMaterialSet *pSet, EGame Version, IOutputStream& rOut);
    static void WriteCookedMaterial(CMaterial *pMat, EGame Version, IOutputStream& rOut);
};

#endif // CMATERIALCOOKER_H
