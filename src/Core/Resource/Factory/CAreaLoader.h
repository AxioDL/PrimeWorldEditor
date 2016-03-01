#ifndef CAREALOADER_H
#define CAREALOADER_H

#include "CSectionMgrIn.h"
#include "Core/Resource/Script/SLink.h"
#include "Core/Resource/CGameArea.h"
#include "Core/Resource/EGame.h"
#include "Core/Resource/CResCache.h"

#include <FileIO/FileIO.h>

class CAreaLoader
{
    struct SCompressedCluster;

    // Area data
    TResPtr<CGameArea> mpArea;
    IInputStream *mpMREA;
    CSectionMgrIn *mpSectionMgr;
    EGame mVersion;
    u32 mNumMeshes;
    u32 mNumLayers;

    // Object connections
    std::unordered_map<u32, std::vector<SLink>> mConnectionMap;

    // Compression
    u8 *mDecmpBuffer;
    bool mHasDecompressedBuffer;
    std::vector<SCompressedCluster> mClusters;
    u32 mTotalDecmpSize;

    // Block numbers
    u32 mGeometryBlockNum;
    u32 mScriptLayerBlockNum;
    u32 mCollisionBlockNum;
    u32 mUnknownBlockNum;
    u32 mLightsBlockNum;
    u32 mEmptyBlockNum;
    u32 mPathBlockNum;
    u32 mOctreeBlockNum;
    u32 mScriptGeneratorBlockNum;
    u32 mFFFFBlockNum;
    u32 mUnknown2BlockNum;
    u32 mEGMCBlockNum;
    u32 mBoundingBoxesBlockNum;
    u32 mDependenciesBlockNum;
    u32 mGPUBlockNum;
    u32 mPVSBlockNum;
    u32 mRSOBlockNum;

    struct SCompressedCluster {
        u32 BufferSize, DecompressedSize, CompressedSize, NumSections;
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
    void ReadLightsCorruption();

    // Common
    void ReadCompressedBlocks();
    void Decompress();
    void LoadSectionDataBuffers();
    void ReadCollision();
    void ReadEGMC();
    void SetUpObjects();

public:
    static CGameArea* LoadMREA(IInputStream& MREA);
    static EGame GetFormatVersion(u32 version);
};

#endif // CAREALOADER_H
