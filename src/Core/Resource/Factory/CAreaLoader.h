#ifndef CAREALOADER_H
#define CAREALOADER_H

#include "CSectionMgrIn.h"
#include "Core/GameProject/CResourceStore.h"
#include "Core/Resource/Area/CGameArea.h"
#include "Core/Resource/Script/CLink.h"
#include <Common/EGame.h>
#include <Common/FileIO.h>
#include <memory>

class CAreaLoader
{
    struct SCompressedCluster;

    // Area data
    TResPtr<CGameArea> mpArea;
    IInputStream *mpMREA = nullptr;
    CSectionMgrIn *mpSectionMgr = nullptr;
    EGame mVersion{};
    uint32 mNumMeshes = 0;
    uint32 mNumLayers = 0;

    // Object connections
    std::unordered_map<uint32, std::vector<CLink*>> mConnectionMap;

    // Compression
    uint8 *mpDecmpBuffer = nullptr;
    bool mHasDecompressedBuffer = false;
    std::vector<SCompressedCluster> mClusters;
    uint32 mTotalDecmpSize = 0;

    // Block numbers
    uint32 mGeometryBlockNum = UINT32_MAX;
    uint32 mScriptLayerBlockNum = UINT32_MAX;
    uint32 mCollisionBlockNum = UINT32_MAX;
    uint32 mUnknownBlockNum = UINT32_MAX;
    uint32 mLightsBlockNum = UINT32_MAX;
    uint32 mVisiBlockNum = UINT32_MAX;
    uint32 mPathBlockNum = UINT32_MAX;
    uint32 mOctreeBlockNum = UINT32_MAX;
    uint32 mScriptGeneratorBlockNum = UINT32_MAX;
    uint32 mFFFFBlockNum = UINT32_MAX;
    uint32 mPTLABlockNum = UINT32_MAX;
    uint32 mEGMCBlockNum = UINT32_MAX;
    uint32 mBoundingBoxesBlockNum = UINT32_MAX;
    uint32 mDependenciesBlockNum = UINT32_MAX;
    uint32 mGPUBlockNum = UINT32_MAX;
    uint32 mRSOBlockNum = UINT32_MAX;

    struct SCompressedCluster {
        uint32 BufferSize;
        uint32 DecompressedSize;
        uint32 CompressedSize;
        uint32 NumSections;
    };

    CAreaLoader();
    ~CAreaLoader();

    // Prime
    void ReadHeaderPrime();
    void ReadGeometryPrime();
    void ReadSCLYPrime();
    void ReadLightsPrime();

    // Echoes
    void ReadHeaderEchoes();
    void ReadSCLYEchoes();

    // Corruption
    void ReadHeaderCorruption();
    void ReadGeometryCorruption();
    void ReadDependenciesCorruption();
    void ReadLightsCorruption();

    // Common
    void ReadCompressedBlocks();
    void Decompress();
    void LoadSectionDataBuffers();
    void ReadCollision();
    void ReadPATH();
    void ReadPTLA();
    void ReadEGMC();
    void SetUpObjects(CScriptLayer *pGenLayer);

public:
    static std::unique_ptr<CGameArea> LoadMREA(IInputStream& rMREA, CResourceEntry *pEntry);
    static EGame GetFormatVersion(uint32 Version);
};

#endif // CAREALOADER_H
