#ifndef CMATERIALCOOKER_H
#define CMATERIALCOOKER_H

#include "../CMaterial.h"
#include "../CMaterialSet.h"
#include "../EFormatVersion.h"

class CMaterialCooker
{
    CMaterialSet *mpSet;
    CMaterial *mpMat;
    EGame mVersion;
    std::vector<u32> mTextureIDs;
    std::vector<u64> mMaterialHashes;

    CMaterialCooker();
    void WriteMatSetPrime(COutputStream& Out);
    void WriteMatSetCorruption(COutputStream& Out);
    void WriteMaterialPrime(COutputStream& Out);
    void WriteMaterialCorruption(COutputStream& Out);

public:
    static void WriteCookedMatSet(CMaterialSet *pSet, EGame Version, COutputStream& Out);
    static void WriteCookedMaterial(CMaterial *pMat, EGame Version, COutputStream& Out);
};

#endif // CMATERIALCOOKER_H
