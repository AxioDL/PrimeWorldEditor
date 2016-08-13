#ifndef CMATERIALLOADER_H
#define CMATERIALLOADER_H

#include "Core/GameProject/CResourceStore.h"
#include "Core/Resource/CMaterialSet.h"
#include <Common/EGame.h>

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
    FVertexDescription ConvertToVertexDescription(u32 VertexFlags);

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
