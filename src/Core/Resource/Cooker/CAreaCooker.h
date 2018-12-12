#ifndef CAREACOOKER_H
#define CAREACOOKER_H

#include "CSectionMgrOut.h"
#include "Core/Resource/Area/CGameArea.h"
#include <Common/EGame.h>
#include <Common/FileIO.h>

class CAreaCooker
{
    TResPtr<CGameArea> mpArea;
    EGame mVersion;

    std::vector<uint32> mSectionSizes;

    uint32 mGeometrySecNum;
    uint32 mSCLYSecNum;
    uint32 mSCGNSecNum;
    uint32 mCollisionSecNum;
    uint32 mUnknownSecNum;
    uint32 mLightsSecNum;
    uint32 mVISISecNum;
    uint32 mPATHSecNum;
    uint32 mAROTSecNum;
    uint32 mFFFFSecNum;
    uint32 mPTLASecNum;
    uint32 mEGMCSecNum;
    uint32 mDepsSecNum;
    uint32 mModulesSecNum;

    struct SCompressedBlock
    {
        uint32 CompressedSize;
        uint32 DecompressedSize;
        uint32 NumSections;

        SCompressedBlock()
            : CompressedSize(0), DecompressedSize(0), NumSections(0) {}
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
