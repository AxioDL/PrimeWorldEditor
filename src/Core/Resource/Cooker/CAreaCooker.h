#ifndef CAREACOOKER_H
#define CAREACOOKER_H

#include "CSectionMgrOut.h"
#include "Core/Resource/Area/CGameArea.h"
#include <Common/EGame.h>
#include <Common/FileIO.h>

class CAreaCooker
{
    TResPtr<CGameArea> mpArea;
    EGame mVersion{};

    std::vector<uint32> mSectionSizes;

    uint32 mGeometrySecNum = UINT32_MAX;
    uint32 mSCLYSecNum = UINT32_MAX;
    uint32 mSCGNSecNum = UINT32_MAX;
    uint32 mCollisionSecNum = UINT32_MAX;
    uint32 mUnknownSecNum = UINT32_MAX;
    uint32 mLightsSecNum = UINT32_MAX;
    uint32 mVISISecNum = UINT32_MAX;
    uint32 mPATHSecNum = UINT32_MAX;
    uint32 mAROTSecNum = UINT32_MAX;
    uint32 mFFFFSecNum = UINT32_MAX;
    uint32 mPTLASecNum = UINT32_MAX;
    uint32 mEGMCSecNum = UINT32_MAX;
    uint32 mDepsSecNum = UINT32_MAX;
    uint32 mModulesSecNum = UINT32_MAX;

    struct SCompressedBlock
    {
        uint32 CompressedSize = 0;
        uint32 DecompressedSize = 0;
        uint32 NumSections = 0;
    };

    SCompressedBlock mCurBlock;
    CVectorOutStream mSectionData;
    CVectorOutStream mCompressedData;
    CVectorOutStream mAreaData;

    std::vector<SCompressedBlock> mCompressedBlocks;

    CAreaCooker();
    void DetermineSectionNumbersPrime();
    void DetermineSectionNumbersCorruption();

    // Header
    void WritePrimeHeader(IOutputStream& rOut);
    void WriteCorruptionHeader(IOutputStream& rOut);
    void WriteCompressionHeader(IOutputStream& rOut);
    void WriteAreaData(IOutputStream& rOut);

    // SCLY
    void WritePrimeSCLY(IOutputStream& rOut);
    void WriteEchoesSCLY(IOutputStream& rOut);

    // Other Sections
    void WriteDependencies(IOutputStream& rOut);
    void WriteModules(IOutputStream& rOut);

    // Section Management
    void AddSectionToBlock();
    void FinishSection(bool ForceFinishBlock);
    void FinishBlock();

public:
    static bool CookMREA(CGameArea *pArea, IOutputStream& rOut);
    static uint32 GetMREAVersion(EGame Version);
};

#endif // CAREACOOKER_H
