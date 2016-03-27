#ifndef CMATERIALCOOKER_H
#define CMATERIALCOOKER_H

#include "Core/Resource/CMaterial.h"
#include "Core/Resource/CMaterialSet.h"
#include "Core/Resource/EGame.h"

class CMaterialCooker
{
    CMaterialSet *mpSet;
    CMaterial *mpMat;
    EGame mVersion;
    std::vector<u32> mTextureIDs;
    std::vector<u64> mMaterialHashes;

    CMaterialCooker();
    void WriteMatSetPrime(IOutputStream& rOut);
    void WriteMatSetCorruption(IOutputStream& rOut);
    void WriteMaterialPrime(IOutputStream& rOut);
    void WriteMaterialCorruption(IOutputStream& rOut);

public:
    static void WriteCookedMatSet(CMaterialSet *pSet, EGame Version, IOutputStream& rOut);
    static void WriteCookedMaterial(CMaterial *pMat, EGame Version, IOutputStream& rOut);
};

#endif // CMATERIALCOOKER_H
