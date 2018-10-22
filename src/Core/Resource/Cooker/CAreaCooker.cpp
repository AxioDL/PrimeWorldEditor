#include "CAreaCooker.h"
#include "CScriptCooker.h"
#include "Core/CompressionUtil.h"
#include "Core/GameProject/DependencyListBuilders.h"
#include <Common/Log.h>

const bool gkForceDisableCompression = false;

CAreaCooker::CAreaCooker()
    : mGeometrySecNum(-1)
    , mSCLYSecNum(-1)
    , mSCGNSecNum(-1)
    , mCollisionSecNum(-1)
    , mUnknownSecNum(-1)
    , mLightsSecNum(-1)
    , mVISISecNum(-1)
    , mPATHSecNum(-1)
    , mAROTSecNum(-1)
    , mFFFFSecNum(-1)
    , mPTLASecNum(-1)
    , mEGMCSecNum(-1)
    , mDepsSecNum(-1)
    , mModulesSecNum(-1)
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
    case EGame::PrimeDemo:
    case EGame::Prime:
        GeometrySections = 1 + (7 * OriginalMeshCount); // Accounting for materials
        break;
    case EGame::EchoesDemo:
        GeometrySections = 2 + (9 * OriginalMeshCount); // Account for materials + AROT
        break;
    case EGame::Echoes:
        GeometrySections = 3 + (9 * OriginalMeshCount); // Acount for materials + AROT + an unknown section
        break;
    }

    for (u32 iMesh = 0; iMesh < mpArea->mWorldModels.size(); iMesh++)
        GeometrySections += mpArea->mWorldModels[iMesh]->GetSurfaceCount();

    // Set section numbers
    u32 SecNum = GeometrySections;
    if (mVersion <= EGame::Prime) mAROTSecNum = SecNum++;
    if (mVersion >= EGame::EchoesDemo) mFFFFSecNum = SecNum++;

    if (mVersion >= EGame::EchoesDemo)
    {
        mSCLYSecNum = SecNum;
        SecNum += (mVersion >= EGame::Echoes ? mpArea->mScriptLayers.size() : 1);
        mSCGNSecNum = SecNum++;
    }
    else
        mSCLYSecNum = SecNum++;

    mCollisionSecNum = SecNum++;
    mUnknownSecNum = SecNum++;
    mLightsSecNum = SecNum++;
    mVISISecNum = SecNum++;
    mPATHSecNum = SecNum++;

    if (mVersion >= EGame::EchoesDemo)
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
        if      (rNum.SectionID == "SOBJ") mSCLYSecNum = rNum.Index;
        else if (rNum.SectionID == "SGEN") mSCGNSecNum = rNum.Index;
        else if (rNum.SectionID == "DEPS") mDepsSecNum = rNum.Index;
        else if (rNum.SectionID == "RSOS") mModulesSecNum = rNum.Index;
    }
}

// ************ HEADER ************
void CAreaCooker::WritePrimeHeader(IOutputStream& rOut)
{
    rOut.WriteLong(0xDEADBEEF);
    rOut.WriteLong(GetMREAVersion(mVersion));
    mpArea->mTransform.Write(rOut);
    rOut.WriteLong(mpArea->mOriginalWorldMeshCount);
    if (mVersion >= EGame::Echoes) rOut.WriteLong(mpArea->mScriptLayers.size());
    rOut.WriteLong(mpArea->mSectionDataBuffers.size());

    rOut.WriteLong(mGeometrySecNum);
    rOut.WriteLong(mSCLYSecNum);
    if (mVersion >= EGame::EchoesDemo) rOut.WriteLong(mSCGNSecNum);
    rOut.WriteLong(mCollisionSecNum);
    rOut.WriteLong(mUnknownSecNum);
    rOut.WriteLong(mLightsSecNum);
    rOut.WriteLong(mVISISecNum);
    rOut.WriteLong(mPATHSecNum);
    if (mVersion <= EGame::Prime) rOut.WriteLong(mAROTSecNum);

    else
    {
        rOut.WriteLong(mFFFFSecNum);
        rOut.WriteLong(mPTLASecNum);
        rOut.WriteLong(mEGMCSecNum);
    }

    if (mVersion >= EGame::EchoesDemo)
    {
        if (mVersion >= EGame::Echoes) rOut.WriteLong(mCompressedBlocks.size());
        rOut.WriteToBoundary(32, 0);
    }

    for (u32 iSec = 0; iSec < mSectionSizes.size(); iSec++)
        rOut.WriteLong(mSectionSizes[iSec]);
    rOut.WriteToBoundary(32, 0);

    if (mVersion >= EGame::Echoes)
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
    // This function covers both Prime 1 and the Echoes demo.
    // The Echoes demo has a similar SCLY format but with minor layout differences and with SCGN.
    rOut.WriteFourCC( FOURCC('SCLY') );
    mVersion <= EGame::Prime ? rOut.WriteLong(1) : rOut.WriteByte(1);

    u32 NumLayers = mpArea->mScriptLayers.size();
    rOut.WriteLong(NumLayers);

    u32 LayerSizesStart = rOut.Tell();
    for (u32 LayerIdx = 0; LayerIdx < NumLayers; LayerIdx++)
        rOut.WriteLong(0);

    // SCLY
    CScriptCooker ScriptCooker(mVersion, true);
    std::vector<u32> LayerSizes(NumLayers);

    for (u32 LayerIdx = 0; LayerIdx < NumLayers; LayerIdx++)
    {
        u32 LayerStart = rOut.Tell();
        ScriptCooker.WriteLayer(rOut, mpArea->mScriptLayers[LayerIdx]);

        // Pad the layer to 32 bytes
        u32 LayerSize = rOut.Tell() - LayerStart;
        u32 PaddedSize = (LayerSize + 31) & ~31;
        u32 NumPadBytes = PaddedSize - LayerSize;

        for (u32 Pad = 0; Pad < NumPadBytes; Pad++)
            rOut.WriteByte(0);

        LayerSizes[LayerIdx] = PaddedSize;
    }

    u32 LayersEnd = rOut.Tell();
    rOut.Seek(LayerSizesStart, SEEK_SET);

    for (u32 LayerIdx = 0; LayerIdx < NumLayers; LayerIdx++)
        rOut.WriteLong(LayerSizes[LayerIdx]);

    rOut.Seek(LayersEnd, SEEK_SET);
    FinishSection(false);

    // SCGN
    if (mVersion == EGame::EchoesDemo)
    {
        rOut.WriteFourCC( FOURCC('SCGN') );
        rOut.WriteByte(1);
        ScriptCooker.WriteGeneratedLayer(rOut);
        FinishSection(false);
    }
}

void CAreaCooker::WriteEchoesSCLY(IOutputStream& rOut)
{
    // SCLY
    CScriptCooker ScriptCooker(mVersion);

    for (u32 LayerIdx = 0; LayerIdx < mpArea->mScriptLayers.size(); LayerIdx++)
    {
        rOut.WriteFourCC( FOURCC('SCLY') );
        rOut.WriteByte(1);
        rOut.WriteLong(LayerIdx);
        ScriptCooker.WriteLayer(rOut, mpArea->mScriptLayers[LayerIdx]);
        FinishSection(true);
    }

    // SCGN
    rOut.WriteFourCC( FOURCC('SCGN') );
    rOut.WriteByte(1);
    ScriptCooker.WriteGeneratedLayer(rOut);
    FinishSection(true);
}

void CAreaCooker::WriteDependencies(IOutputStream& rOut)
{
    // Build dependency list
    std::list<CAssetID> Dependencies;
    std::list<u32> LayerOffsets;

    CAreaDependencyListBuilder Builder(mpArea->Entry());
    Builder.BuildDependencyList(Dependencies, LayerOffsets);

    // Write
    rOut.WriteLong(Dependencies.size());

    for (auto Iter = Dependencies.begin(); Iter != Dependencies.end(); Iter++)
    {
        CAssetID ID = *Iter;
        CResourceEntry *pEntry = gpResourceStore->FindEntry(ID);
        ID.Write(rOut);
        pEntry->CookedExtension().Write(rOut);
    }

    rOut.WriteLong(LayerOffsets.size());

    for (auto Iter = LayerOffsets.begin(); Iter != LayerOffsets.end(); Iter++)
        rOut.WriteLong(*Iter);

    FinishSection(false);
}

void CAreaCooker::WriteModules(IOutputStream& rOut)
{
    // Build module list
    std::vector<TString> ModuleNames;
    std::vector<u32> LayerOffsets;

    CAreaDependencyTree *pAreaDeps = static_cast<CAreaDependencyTree*>(mpArea->Entry()->Dependencies());
    pAreaDeps->GetModuleDependencies(mpArea->Game(), ModuleNames, LayerOffsets);

    // Write
    rOut.WriteLong(ModuleNames.size());

    for (u32 ModuleIdx = 0; ModuleIdx < ModuleNames.size(); ModuleIdx++)
        rOut.WriteString( ModuleNames[ModuleIdx] );

    rOut.WriteLong(LayerOffsets.size());

    for (u32 OffsetIdx = 0; OffsetIdx < LayerOffsets.size(); OffsetIdx++)
        rOut.WriteLong(LayerOffsets[OffsetIdx]);

    FinishSection(false);
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
    if (mVersion >= EGame::Echoes)
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
    bool EnableCompression = (mVersion >= EGame::Echoes) && mpArea->mUsesCompression && !gkForceDisableCompression;
    bool UseZlib = (mVersion == EGame::DKCReturns);

    u32 CompressedSize = 0;
    bool WriteCompressedData = false;

    if (EnableCompression)
    {
        bool Success = CompressionUtil::CompressSegmentedData((u8*) mCompressedData.Data(), mCompressedData.Size(), CompressedBuf.data(), CompressedSize, UseZlib, true);
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
bool CAreaCooker::CookMREA(CGameArea *pArea, IOutputStream& rOut)
{
    CAreaCooker Cooker;
    Cooker.mpArea = pArea;
    Cooker.mVersion = pArea->Game();

    if (Cooker.mVersion <= EGame::Echoes)
        Cooker.DetermineSectionNumbersPrime();
    else
        Cooker.DetermineSectionNumbersCorruption();

    // Write pre-SCLY data sections
    for (u32 iSec = 0; iSec < Cooker.mSCLYSecNum; iSec++)
    {
        if (iSec == Cooker.mDepsSecNum)
            Cooker.WriteDependencies(Cooker.mSectionData);

        else
        {
            Cooker.mSectionData.WriteBytes(pArea->mSectionDataBuffers[iSec].data(), pArea->mSectionDataBuffers[iSec].size());
            Cooker.FinishSection(false);
        }
    }

    // Write SCLY
    if (Cooker.mVersion <= EGame::EchoesDemo)
        Cooker.WritePrimeSCLY(Cooker.mSectionData);
    else
        Cooker.WriteEchoesSCLY(Cooker.mSectionData);

    // Write post-SCLY data sections
    u32 PostSCLY = (Cooker.mVersion <= EGame::Prime ? Cooker.mSCLYSecNum + 1 : Cooker.mSCGNSecNum + 1);
    for (u32 iSec = PostSCLY; iSec < pArea->mSectionDataBuffers.size(); iSec++)
    {
        if (iSec == Cooker.mModulesSecNum)
            Cooker.WriteModules(Cooker.mSectionData);

        else
        {
            Cooker.mSectionData.WriteBytes(pArea->mSectionDataBuffers[iSec].data(), pArea->mSectionDataBuffers[iSec].size());
            Cooker.FinishSection(false);
        }
    }

    Cooker.FinishBlock();

    // Write to actual file
    if (Cooker.mVersion <= EGame::Echoes)
        Cooker.WritePrimeHeader(rOut);
    else
        Cooker.WriteCorruptionHeader(rOut);

    Cooker.WriteAreaData(rOut);
    return true;
}

u32 CAreaCooker::GetMREAVersion(EGame Version)
{
    switch (Version)
    {
    case EGame::PrimeDemo:          return 0xC;
    case EGame::Prime:              return 0xF;
    case EGame::EchoesDemo:         return 0x15;
    case EGame::Echoes:             return 0x19;
    case EGame::CorruptionProto:    return 0x1D;
    case EGame::Corruption:         return 0x1E;
    case EGame::DKCReturns:         return 0x20;
    default:                        return 0;
    }
}
