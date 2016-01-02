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
    void WriteMatSetPrime(IOutputStream& Out);
    void WriteMatSetCorruption(IOutputStream& Out);
    void WriteMaterialPrime(IOutputStream& Out);
    void WriteMaterialCorruption(IOutputStream& Out);

public:
    static void WriteCookedMatSet(CMaterialSet *pSet, EGame Version, IOutputStream& Out);
    static void WriteCookedMaterial(CMaterial *pMat, EGame Version, IOutputStream& Out);
};

#endif // CMATERIALCOOKER_H
