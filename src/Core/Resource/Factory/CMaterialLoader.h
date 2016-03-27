#ifndef CMATERIALLOADER_H
#define CMATERIALLOADER_H

#include "Core/Resource/CMaterialSet.h"
#include "Core/Resource/EGame.h"
#include "Core/Resource/CResCache.h"

#include <FileIO/FileIO.h>
#include <assimp/scene.h>

class CMaterialLoader
{
    // Material data
    CMaterialSet *mpSet;
    IInputStream *mpFile;
    EGame mVersion;
    std::vector<TResPtr<CTexture>> mTextures;
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

    CMaterial* LoadAssimpMaterial(const aiMaterial *pAiMat);

    // Static
public:
    static CMaterialSet* LoadMaterialSet(IInputStream& rMat, EGame Version);
    static CMaterialSet* ImportAssimpMaterials(const aiScene *pScene, EGame TargetVersion);
};

#endif // CMATERIALLOADER_H
