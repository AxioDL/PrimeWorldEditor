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
    uint32 mNumMeshes;
    uint32 mNumLayers;

    // Object connections
    std::unordered_map<uint32, std::vector<CLink*>> mConnectionMap;

    // Compression
    uint8 *mpDecmpBuffer;
    bool mHasDecompressedBuffer;
    std::vector<SCompressedCluster> mClusters;
    uint32 mTotalDecmpSize;

    // Block numbers
    uint32 mGeometryBlockNum;
    uint32 mScriptLayerBlockNum;
    uint32 mCollisionBlockNum;
    uint32 mUnknownBlockNum;
    uint32 mLightsBlockNum;
    uint32 mVisiBlockNum;
    uint32 mPathBlockNum;
    uint32 mOctreeBlockNum;
    uint32 mScriptGeneratorBlockNum;
    uint32 mFFFFBlockNum;
    uint32 mPTLABlockNum;
    uint32 mEGMCBlockNum;
    uint32 mBoundingBoxesBlockNum;
    uint32 mDependenciesBlockNum;
    uint32 mGPUBlockNum;
    uint32 mRSOBlockNum;

    struct SCompressedCluster {
        uint32 BufferSize, DecompressedSize, CompressedSize, NumSections;
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
    static EGame GetFormatVersion(uint32 Version);
};

#endif // CAREALOADER_H
