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

    std::vector<u32> mSectionSizes;

    u32 mGeometrySecNum;
    u32 mSCLYSecNum;
    u32 mSCGNSecNum;
    u32 mCollisionSecNum;
    u32 mUnknownSecNum;
    u32 mLightsSecNum;
    u32 mVISISecNum;
    u32 mPATHSecNum;
    u32 mAROTSecNum;
    u32 mFFFFSecNum;
    u32 mPTLASecNum;
    u32 mEGMCSecNum;
    u32 mDepsSecNum;
    u32 mModulesSecNum;

    struct SCompressedBlock
    {
        u32 CompressedSize;
        u32 DecompressedSize;
        u32 NumSections;

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
    static u32 GetMREAVersion(EGame Version);
};

#endif // CAREACOOKER_H
