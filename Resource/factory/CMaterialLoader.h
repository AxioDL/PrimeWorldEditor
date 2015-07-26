#ifndef CMATERIALLOADER_H
#define CMATERIALLOADER_H

#include <FileIO/FileIO.h>
#include "../CMaterialSet.h"
#include "../EFormatVersion.h"
#include <Core/CResCache.h>

class CMaterialLoader
{
    // Material data
    CMaterialSet *mpSet;
    CInputStream *mpFile;
    EGame mVersion;
    bool mHasOPAC;
    bool mHas0x400;

    CColor mCorruptionColors[4];
    u8 mCorruptionInts[5];
    u32 mCorruptionFlags;
    std::vector<u32> mPassOffsets;

    CMaterialLoader();
    ~CMaterialLoader();

    // Load Functions
    void ReadPrimeMatSet();
    CMaterial* ReadPrimeMaterial();

    void ReadCorruptionMatSet();
    CMaterial* ReadCorruptionMaterial();
    void CreateCorruptionPasses(CMaterial *pMat);

public:
    static CMaterialSet* LoadMaterialSet(CInputStream& Mat, EGame Version);
};

#endif // CMATERIALLOADER_H
