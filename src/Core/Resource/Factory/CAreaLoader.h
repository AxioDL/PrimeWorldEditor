#ifndef CAREALOADER_H
#define CAREALOADER_H

#include "CSectionMgrIn.h"
#include "Core/GameProject/CResourceStore.h"
#include "Core/Resource/Area/CGameArea.h"
#include "Core/Resource/Script/CLink.h"
#include <Common/EGame.h>
#include <Common/FileIO.h>

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
    std::unordered_map<u32, std::vector<CLink*>> mConnectionMap;

    // Compression
    u8 *mpDecmpBuffer;
    bool mHasDecompressedBuffer;
    std::vector<SCompressedCluster> mClusters;
    u32 mTotalDecmpSize;

    // Block numbers
    u32 mGeometryBlockNum;
    u32 mScriptLayerBlockNum;
    u32 mCollisionBlockNum;
    u32 mUnknownBlockNum;
    u32 mLightsBlockNum;
    u32 mVisiBlockNum;
    u32 mPathBlockNum;
    u32 mOctreeBlockNum;
    u32 mScriptGeneratorBlockNum;
    u32 mFFFFBlockNum;
    u32 mPTLABlockNum;
    u32 mEGMCBlockNum;
    u32 mBoundingBoxesBlockNum;
    u32 mDependenciesBlockNum;
    u32 mGPUBlockNum;
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
    static CGameArea* LoadMREA(IInputStream& rMREA, CResourceEntry *pEntry);
    static EGame GetFormatVersion(u32 Version);
};

#endif // CAREALOADER_H
