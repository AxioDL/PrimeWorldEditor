#ifndef CAREALOADER_H
#define CAREALOADER_H

#include <Common/EGame.h>
#include <Core/Resource/TResPtr.h>
#include <Core/Resource/Script/CInstanceID.h>

#include <memory>
#include <unordered_map>
#include <vector>

class CGameArea;
class CLink;
class CResourceEntry;
class CSectionMgrIn;
class IInputStream;

class CAreaLoader
{
    struct SCompressedCluster;

    // Area data
    TResPtr<CGameArea> mpArea;
    IInputStream *mpMREA = nullptr;
    std::unique_ptr<CSectionMgrIn> mpSectionMgr;
    EGame mVersion{};
    uint32_t mNumMeshes = 0;
    uint32_t mNumLayers = 0;

    // Object connections
    std::unordered_map<CInstanceID, std::vector<CLink*>> mConnectionMap;

    // Compression
    std::unique_ptr<uint8_t[]> mpDecmpBuffer;
    bool mHasDecompressedBuffer = false;
    std::vector<SCompressedCluster> mClusters;
    uint32_t mTotalDecmpSize = 0;

    // Block numbers
    uint32_t mGeometryBlockNum = UINT32_MAX;
    uint32_t mScriptLayerBlockNum = UINT32_MAX;
    uint32_t mCollisionBlockNum = UINT32_MAX;
    uint32_t mUnknownBlockNum = UINT32_MAX;
    uint32_t mLightsBlockNum = UINT32_MAX;
    uint32_t mVisiBlockNum = UINT32_MAX;
    uint32_t mPathBlockNum = UINT32_MAX;
    uint32_t mOctreeBlockNum = UINT32_MAX;
    uint32_t mScriptGeneratorBlockNum = UINT32_MAX;
    uint32_t mFFFFBlockNum = UINT32_MAX;
    uint32_t mPTLABlockNum = UINT32_MAX;
    uint32_t mEGMCBlockNum = UINT32_MAX;
    uint32_t mBoundingBoxesBlockNum = UINT32_MAX;
    uint32_t mDependenciesBlockNum = UINT32_MAX;
    uint32_t mGPUBlockNum = UINT32_MAX;
    uint32_t mRSOBlockNum = UINT32_MAX;

    struct SCompressedCluster {
        uint32_t BufferSize{};
        uint32_t DecompressedSize{};
        uint32_t CompressedSize{};
        uint32_t NumSections{};
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
    static std::unique_ptr<CGameArea> LoadMREA(IInputStream& rMREA, CResourceEntry *pEntry, bool GeometryOnly = false);
    static EGame GetFormatVersion(uint32_t Version);
};

#endif // CAREALOADER_H
