#include "CAreaCooker.h"
#include "CScriptCooker.h"
#include <Common/CompressionUtil.h>
#include <Common/Log.h>

const bool gkForceDisableCompression = false;

CAreaCooker::CAreaCooker()
{
}

void CAreaCooker::DetermineSectionNumbersPrime()
{
    mGeometrySecNum = 0;

    // Determine how many sections are taken up by geometry...
    // Each world mesh has 7-9 sections (depending on game) plus one section per surface.
    u32 GeometrySections = 0;
    u32 OriginalMeshCount = mpArea->mOriginalWorldMeshCount;

    switch (mVersion)
    {
    case ePrimeDemo:
    case ePrime:
        GeometrySections = 1 + (7 * OriginalMeshCount); // Accounting for materials
        break;
    case eEchoesDemo:
        GeometrySections = 2 + (9 * OriginalMeshCount); // Account for materials + AROT
        break;
    case eEchoes:
        GeometrySections = 3 + (9 * OriginalMeshCount); // Acount for materials + AROT + an unknown section
        break;
    }

    for (u32 iMesh = 0; iMesh < mpArea->mTerrainModels.size(); iMesh++)
        GeometrySections += mpArea->mTerrainModels[iMesh]->GetSurfaceCount();

    // Set section numbers
    u32 SecNum = GeometrySections;
    if (mVersion <= ePrime) mAROTSecNum = SecNum++;
    if (mVersion >= eEchoesDemo) mFFFFSecNum = SecNum++;

    if (mVersion >= eEchoesDemo)
    {
        mSCLYSecNum = SecNum;
        SecNum += (mVersion >= eEchoes ? mpArea->mScriptLayers.size() : 1);
        mSCGNSecNum = SecNum++;
    }
    else
        mSCLYSecNum = SecNum++;

    mCollisionSecNum = SecNum++;
    mUnknownSecNum = SecNum++;
    mLightsSecNum = SecNum++;
    mVISISecNum = SecNum++;
    mPATHSecNum = SecNum++;

    if (mVersion >= eEchoesDemo)
    {
        mPTLASecNum = SecNum++;
        mEGMCSecNum = SecNum++;
    }
}

void CAreaCooker::DetermineSectionNumbersCorruption()
{
    // Because we're copying these from the original file (because not all the numbers
    // are present in every file), we don't care about any of these except SCLY and SCGN.
    for (u32 iNum = 0; iNum < mpArea->mSectionNumbers.size(); iNum++)
    {
        CGameArea::SSectionNumber& rNum = mpArea->mSectionNumbers[iNum];
        if (rNum.SectionID == "SOBJ") mSCLYSecNum = rNum.Index;
        else if (rNum.SectionID == "SGEN") mSCGNSecNum = rNum.Index;
    }
}

// ************ HEADER ************
void CAreaCooker::WritePrimeHeader(IOutputStream& rOut)
{
    rOut.WriteLong(0xDEADBEEF);
    rOut.WriteLong(GetMREAVersion(mVersion));
    mpArea->mTransform.Write(rOut);
    rOut.WriteLong(mpArea->mOriginalWorldMeshCount);
    if (mVersion >= eEchoes) rOut.WriteLong(mpArea->mScriptLayers.size());
    rOut.WriteLong(mpArea->mSectionDataBuffers.size());

    rOut.WriteLong(mGeometrySecNum);
    rOut.WriteLong(mSCLYSecNum);
    if (mVersion >= eEchoesDemo) rOut.WriteLong(mSCGNSecNum);
    rOut.WriteLong(mCollisionSecNum);
    rOut.WriteLong(mUnknownSecNum);
    rOut.WriteLong(mLightsSecNum);
    rOut.WriteLong(mVISISecNum);
    rOut.WriteLong(mPATHSecNum);
    if (mVersion <= ePrime) rOut.WriteLong(mAROTSecNum);

    else
    {
        rOut.WriteLong(mFFFFSecNum);
        rOut.WriteLong(mPTLASecNum);
        rOut.WriteLong(mEGMCSecNum);
    }

    if (mVersion >= eEchoesDemo)
    {
        if (mVersion >= eEchoes) rOut.WriteLong(mCompressedBlocks.size());
        rOut.WriteToBoundary(32, 0);
    }

    for (u32 iSec = 0; iSec < mSectionSizes.size(); iSec++)
        rOut.WriteLong(mSectionSizes[iSec]);
    rOut.WriteToBoundary(32, 0);

    if (mVersion >= eEchoes)
        WriteCompressionHeader(rOut);
}

void CAreaCooker::WriteCorruptionHeader(IOutputStream& rOut)
{
    rOut.WriteLong(0xDEADBEEF);
    rOut.WriteLong(GetMREAVersion(mVersion));
    mpArea->mTransform.Write(rOut);
    rOut.WriteLong(mpArea->mOriginalWorldMeshCount);
    rOut.WriteLong(mpArea->mScriptLayers.size());
    rOut.WriteLong(mpArea->mSectionDataBuffers.size());
    rOut.WriteLong(mCompressedBlocks.size());
    rOut.WriteLong(mpArea->mSectionNumbers.size());
    rOut.WriteToBoundary(32, 0);

    for (u32 iSec = 0; iSec < mSectionSizes.size(); iSec++)
        rOut.WriteLong(mSectionSizes[iSec]);

    rOut.WriteToBoundary(32, 0);

    WriteCompressionHeader(rOut);

    for (u32 iNum = 0; iNum < mpArea->mSectionNumbers.size(); iNum++)
    {
        CGameArea::SSectionNumber& rNum = mpArea->mSectionNumbers[iNum];
        rOut.WriteLong(rNum.SectionID.ToLong());
        rOut.WriteLong(rNum.Index);
    }
    rOut.WriteToBoundary(32, 0);
}

void CAreaCooker::WriteCompressionHeader(IOutputStream& rOut)
{
    for (u32 iCmp = 0; iCmp < mCompressedBlocks.size(); iCmp++)
    {
        SCompressedBlock& rBlock = mCompressedBlocks[iCmp];
        bool IsCompressed = (rBlock.CompressedSize != 0);

        rOut.WriteLong(IsCompressed ? rBlock.DecompressedSize + 0x120 : rBlock.DecompressedSize);
        rOut.WriteLong(rBlock.DecompressedSize);
        rOut.WriteLong(rBlock.CompressedSize);
        rOut.WriteLong(rBlock.NumSections);
    }

    rOut.WriteToBoundary(32, 0);
}

void CAreaCooker::WriteAreaData(IOutputStream& rOut)
{
    rOut.WriteBytes(mAreaData.Data(), mAreaData.Size());
    rOut.WriteToBoundary(32, 0);
}

// ************ SCLY ************
void CAreaCooker::WritePrimeSCLY(IOutputStream& rOut)
{
    rOut.WriteString("SCLY", 4);
    mVersion <= ePrime ? rOut.WriteLong(1) : rOut.WriteByte(1);

    u32 NumLayers = mpArea->mScriptLayers.size();
    rOut.WriteLong(NumLayers);

    u32 LayerSizesStart = rOut.Tell();
    for (u32 iLyr = 0; iLyr < NumLayers; iLyr++)
        rOut.WriteLong(0);

    // SCLY
    std::vector<u32> LayerSizes(NumLayers);

    for (u32 iLyr = 0; iLyr < NumLayers; iLyr++)
    {
        u32 LayerStart = rOut.Tell();
        CScriptCooker::WriteLayer(mVersion, mpArea->mScriptLayers[iLyr], rOut);
        LayerSizes[iLyr] = rOut.Tell() - LayerStart;
    }

    u32 LayersEnd = rOut.Tell();
    rOut.Seek(LayerSizesStart, SEEK_SET);

    for (u32 iLyr = 0; iLyr < NumLayers; iLyr++)
        rOut.WriteLong(LayerSizes[iLyr]);

    rOut.Seek(LayersEnd, SEEK_SET);
    FinishSection(false);

    // SCGN
    if (mVersion == eEchoesDemo)
    {
        rOut.WriteString("SCGN", 4);
        rOut.WriteByte(1);
        CScriptCooker::WriteLayer(mVersion, mpArea->mpGeneratorLayer, rOut);
        FinishSection(false);
    }
}

void CAreaCooker::WriteEchoesSCLY(IOutputStream& rOut)
{
    // SCLY
    for (u32 iLyr = 0; iLyr < mpArea->mScriptLayers.size(); iLyr++)
    {
        rOut.WriteString("SCLY", 4);
        rOut.WriteByte(0x1);
        rOut.WriteLong(iLyr);
        CScriptCooker::WriteLayer(mVersion, mpArea->mScriptLayers[iLyr], rOut);
        FinishSection(true);
    }

    // SCGN
    rOut.WriteString("SCGN", 4);
    rOut.WriteByte(0x1);
    CScriptCooker::WriteLayer(mVersion, mpArea->mpGeneratorLayer, rOut);
    FinishSection(true);
}

// ************ SECTION MANAGEMENT ************
void CAreaCooker::AddSectionToBlock()
{
    mCompressedData.WriteBytes(mSectionData.Data(), mSectionData.Size());
    mCompressedData.WriteToBoundary(32, 0);
    mCurBlock.DecompressedSize += mSectionData.Size();
    mCurBlock.NumSections++;
}

void CAreaCooker::FinishSection(bool SingleSectionBlock)
{
    // Our section data is now finished in mSection...
    const u32 kSizeThreshold = 0x20000;
    mSectionData.WriteToBoundary(32, 0);

    u32 SecSize = mSectionData.Size();
    mSectionSizes.push_back(SecSize);

    // Only track compressed blocks for MP2+. Write everything to one block for MP1.
    if (mVersion >= eEchoes)
    {
        // Finish the current block if this is a single section block OR if the new section would push the block over the size limit.
        if (mCurBlock.NumSections > 0 && (mCurBlock.DecompressedSize + SecSize > kSizeThreshold || SingleSectionBlock))
            FinishBlock();

        AddSectionToBlock();

        // And finally for a single section block, finish the new block.
        if (SingleSectionBlock)
            FinishBlock();
    }

    else AddSectionToBlock();

    mSectionData.Clear();
}

void CAreaCooker::FinishBlock()
{
    if (mCurBlock.NumSections == 0) return;

    std::vector<u8> CompressedBuf(mCompressedData.Size() * 2);
    bool EnableCompression = (mVersion >= eEchoes) && mpArea->mUsesCompression && !gkForceDisableCompression;
    bool UseZlib = (mVersion == eReturns);

    u32 CompressedSize = 0;
    bool WriteCompressedData = false;

    if (EnableCompression)
    {
        bool Success = CompressionUtil::CompressSegmentedData((u8*) mCompressedData.Data(), mCompressedData.Size(), CompressedBuf.data(), CompressedSize, UseZlib);
        u32 PadBytes = (32 - (CompressedSize % 32)) & 0x1F;
        WriteCompressedData = Success && (CompressedSize + PadBytes < (u32) mCompressedData.Size());
    }

    if (WriteCompressedData)
    {
        u32 PadBytes = 32 - (CompressedSize % 32);
        PadBytes &= 0x1F;

        for (u32 iPad = 0; iPad < PadBytes; iPad++)
            mAreaData.WriteByte(0);

        mAreaData.WriteBytes(CompressedBuf.data(), CompressedSize);
        mCurBlock.CompressedSize = CompressedSize;
    }

    else
    {
        mAreaData.WriteBytes(mCompressedData.Data(), mCompressedData.Size());
        mAreaData.WriteToBoundary(32, 0);
        mCurBlock.CompressedSize = 0;
    }

    mCompressedData.Clear();
    mCompressedBlocks.push_back(mCurBlock);
    mCurBlock = SCompressedBlock();
}

// ************ STATIC ************
void CAreaCooker::WriteCookedArea(CGameArea *pArea, IOutputStream& rOut)
{
    CAreaCooker Cooker;
    Cooker.mpArea = pArea;
    Cooker.mVersion = pArea->Version();

    if (Cooker.mVersion <= eEchoes)
        Cooker.DetermineSectionNumbersPrime();
    else
        Cooker.DetermineSectionNumbersCorruption();

    // Write pre-SCLY data sections
    for (u32 iSec = 0; iSec < Cooker.mSCLYSecNum; iSec++)
    {
        Cooker.mSectionData.WriteBytes(pArea->mSectionDataBuffers[iSec].data(), pArea->mSectionDataBuffers[iSec].size());
        Cooker.FinishSection(false);
    }

    // Write SCLY
    if (Cooker.mVersion <= eEchoesDemo)
        Cooker.WritePrimeSCLY(Cooker.mSectionData);
    else
        Cooker.WriteEchoesSCLY(Cooker.mSectionData);

    // Write post-SCLY data sections
    u32 PostSCLY = (Cooker.mVersion <= ePrime ? Cooker.mSCLYSecNum + 1 : Cooker.mSCGNSecNum + 1);
    for (u32 iSec = PostSCLY; iSec < pArea->mSectionDataBuffers.size(); iSec++)
    {
        Cooker.mSectionData.WriteBytes(pArea->mSectionDataBuffers[iSec].data(), pArea->mSectionDataBuffers[iSec].size());
        Cooker.FinishSection(false);
    }

    Cooker.FinishBlock();

    // Write to actual file
    if (Cooker.mVersion <= eEchoes)
        Cooker.WritePrimeHeader(rOut);
    else
        Cooker.WriteCorruptionHeader(rOut);

    Cooker.WriteAreaData(rOut);
}

u32 CAreaCooker::GetMREAVersion(EGame version)
{
    switch (version)
    {
    case ePrimeDemo:       return 0xC;
    case ePrime:           return 0xF;
    case eEchoesDemo:      return 0x15;
    case eEchoes:          return 0x19;
    case eCorruptionProto: return 0x1D;
    case eCorruption:      return 0x1E;
    case eReturns:         return 0x20;
    default:               return 0;
    }
}
