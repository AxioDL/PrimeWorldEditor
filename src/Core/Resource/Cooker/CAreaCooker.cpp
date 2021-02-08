#include "CAreaCooker.h"
#include "CScriptCooker.h"
#include "Core/CompressionUtil.h"
#include "Core/GameProject/DependencyListBuilders.h"
#include <Common/Log.h>

constexpr bool gkForceDisableCompression = false;

CAreaCooker::CAreaCooker() = default;

void CAreaCooker::DetermineSectionNumbersPrime()
{
    mGeometrySecNum = 0;

    // Determine how many sections are taken up by geometry...
    // Each world mesh has 7-9 sections (depending on game) plus one section per surface.
    uint32 GeometrySections = 0;
    const uint32 OriginalMeshCount = mpArea->mOriginalWorldMeshCount;

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
    default:
        break;
    }

    for (const auto& worldModel : mpArea->mWorldModels)
        GeometrySections += worldModel->GetSurfaceCount();

    // Set section numbers
    uint32 SecNum = GeometrySections;
    if (mVersion <= EGame::Prime)
        mAROTSecNum = SecNum++;
    if (mVersion >= EGame::EchoesDemo)
        mFFFFSecNum = SecNum++;

    if (mVersion >= EGame::EchoesDemo)
    {
        mSCLYSecNum = SecNum;
        SecNum += (mVersion >= EGame::Echoes ? mpArea->mScriptLayers.size() : 1);
        mSCGNSecNum = SecNum++;
    }
    else
    {
        mSCLYSecNum = SecNum++;
    }

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
    for (const auto& num : mpArea->mSectionNumbers)
    {
        if (num.SectionID == "SOBJ")
            mSCLYSecNum = num.Index;
        else if (num.SectionID == "SGEN")
            mSCGNSecNum = num.Index;
        else if (num.SectionID == "DEPS")
            mDepsSecNum = num.Index;
        else if (num.SectionID == "RSOS")
            mModulesSecNum = num.Index;
    }
}

// ************ HEADER ************
void CAreaCooker::WritePrimeHeader(IOutputStream& rOut)
{
    rOut.WriteULong(0xDEADBEEF);
    rOut.WriteULong(GetMREAVersion(mVersion));
    mpArea->mTransform.Write(rOut);
    rOut.WriteULong(mpArea->mOriginalWorldMeshCount);
    if (mVersion >= EGame::Echoes)
        rOut.WriteULong(static_cast<uint32>(mpArea->mScriptLayers.size()));
    rOut.WriteULong(static_cast<uint32>(mpArea->mSectionDataBuffers.size()));

    rOut.WriteULong(mGeometrySecNum);
    rOut.WriteULong(mSCLYSecNum);
    if (mVersion >= EGame::EchoesDemo)
        rOut.WriteULong(mSCGNSecNum);
    rOut.WriteULong(mCollisionSecNum);
    rOut.WriteULong(mUnknownSecNum);
    rOut.WriteULong(mLightsSecNum);
    rOut.WriteULong(mVISISecNum);
    rOut.WriteULong(mPATHSecNum);
    if (mVersion <= EGame::Prime)
        rOut.WriteULong(mAROTSecNum);

    else
    {
        rOut.WriteULong(mFFFFSecNum);
        rOut.WriteULong(mPTLASecNum);
        rOut.WriteULong(mEGMCSecNum);
    }

    if (mVersion >= EGame::EchoesDemo)
    {
        if (mVersion >= EGame::Echoes)
            rOut.WriteULong(static_cast<uint32>(mCompressedBlocks.size()));
        rOut.WriteToBoundary(32, 0);
    }

    for (const uint32 size : mSectionSizes)
        rOut.WriteULong(size);
    rOut.WriteToBoundary(32, 0);

    if (mVersion >= EGame::Echoes)
        WriteCompressionHeader(rOut);
}

void CAreaCooker::WriteCorruptionHeader(IOutputStream& rOut)
{
    rOut.WriteULong(0xDEADBEEF);
    rOut.WriteULong(GetMREAVersion(mVersion));
    mpArea->mTransform.Write(rOut);
    rOut.WriteULong(mpArea->mOriginalWorldMeshCount);
    rOut.WriteULong(static_cast<uint32>(mpArea->mScriptLayers.size()));
    rOut.WriteULong(static_cast<uint32>(mpArea->mSectionDataBuffers.size()));
    rOut.WriteULong(static_cast<uint32>(mCompressedBlocks.size()));
    rOut.WriteULong(static_cast<uint32>(mpArea->mSectionNumbers.size()));
    rOut.WriteToBoundary(32, 0);

    for (const uint32 size : mSectionSizes)
        rOut.WriteULong(size);

    rOut.WriteToBoundary(32, 0);

    WriteCompressionHeader(rOut);

    for (const auto& num : mpArea->mSectionNumbers)
    {
        rOut.WriteULong(num.SectionID.ToLong());
        rOut.WriteULong(num.Index);
    }
    rOut.WriteToBoundary(32, 0);
}

void CAreaCooker::WriteCompressionHeader(IOutputStream& rOut)
{
    for (const auto& block : mCompressedBlocks)
    {
        const bool IsCompressed = block.CompressedSize != 0;

        rOut.WriteULong(IsCompressed ? block.DecompressedSize + 0x120 : block.DecompressedSize);
        rOut.WriteULong(block.DecompressedSize);
        rOut.WriteULong(block.CompressedSize);
        rOut.WriteULong(block.NumSections);
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

    const auto NumLayers = static_cast<uint32>(mpArea->mScriptLayers.size());
    rOut.WriteULong(NumLayers);

    const uint32 LayerSizesStart = rOut.Tell();
    for (uint32 LayerIdx = 0; LayerIdx < NumLayers; LayerIdx++)
        rOut.WriteULong(0);

    // SCLY
    CScriptCooker ScriptCooker(mVersion, true);
    std::vector<uint32> LayerSizes(NumLayers);

    for (uint32 LayerIdx = 0; LayerIdx < NumLayers; LayerIdx++)
    {
        const uint32 LayerStart = rOut.Tell();
        ScriptCooker.WriteLayer(rOut, mpArea->mScriptLayers[LayerIdx].get());

        // Pad the layer to 32 bytes
        const uint32 LayerSize = rOut.Tell() - LayerStart;
        const uint32 PaddedSize = (LayerSize + 31) & ~31;
        const uint32 NumPadBytes = PaddedSize - LayerSize;

        for (uint32 Pad = 0; Pad < NumPadBytes; Pad++)
            rOut.WriteByte(0);

        LayerSizes[LayerIdx] = PaddedSize;
    }

    const uint32 LayersEnd = rOut.Tell();
    rOut.Seek(LayerSizesStart, SEEK_SET);

    for (uint32 LayerIdx = 0; LayerIdx < NumLayers; LayerIdx++)
        rOut.WriteULong(LayerSizes[LayerIdx]);

    rOut.Seek(LayersEnd, SEEK_SET);
    FinishSection(false);

    // SCGN
    if (mVersion == EGame::EchoesDemo)
    {
        rOut.WriteFourCC(FOURCC('SCGN'));
        rOut.WriteUByte(1);
        ScriptCooker.WriteGeneratedLayer(rOut);
        FinishSection(false);
    }
}

void CAreaCooker::WriteEchoesSCLY(IOutputStream& rOut)
{
    // SCLY
    CScriptCooker ScriptCooker(mVersion);

    for (uint32 LayerIdx = 0; LayerIdx < mpArea->mScriptLayers.size(); LayerIdx++)
    {
        rOut.WriteFourCC(FOURCC('SCLY'));
        rOut.WriteUByte(1);
        rOut.WriteULong(LayerIdx);
        ScriptCooker.WriteLayer(rOut, mpArea->mScriptLayers[LayerIdx].get());
        FinishSection(true);
    }

    // SCGN
    rOut.WriteFourCC(FOURCC('SCGN'));
    rOut.WriteUByte(1);
    ScriptCooker.WriteGeneratedLayer(rOut);
    FinishSection(true);
}

void CAreaCooker::WriteDependencies(IOutputStream& rOut)
{
    // Build dependency list
    std::list<CAssetID> Dependencies;
    std::list<uint32> LayerOffsets;

    CAreaDependencyListBuilder Builder(mpArea->Entry());
    Builder.BuildDependencyList(Dependencies, LayerOffsets);

    // Write
    rOut.WriteULong(static_cast<uint32>(Dependencies.size()));

    for (const auto& dependency : Dependencies)
    {
        CResourceEntry *pEntry = gpResourceStore->FindEntry(dependency);
        dependency.Write(rOut);
        pEntry->CookedExtension().Write(rOut);
    }

    rOut.WriteULong(static_cast<uint32>(LayerOffsets.size()));

    for (const uint32 offset : LayerOffsets)
        rOut.WriteULong(offset);

    FinishSection(false);
}

void CAreaCooker::WriteModules(IOutputStream& rOut)
{
    // Build module list
    std::vector<TString> ModuleNames;
    std::vector<uint32> LayerOffsets;

    CAreaDependencyTree *pAreaDeps = static_cast<CAreaDependencyTree*>(mpArea->Entry()->Dependencies());
    pAreaDeps->GetModuleDependencies(mpArea->Game(), ModuleNames, LayerOffsets);

    // Write
    rOut.WriteULong(static_cast<uint32>(ModuleNames.size()));

    for (const auto& name : ModuleNames)
        rOut.WriteString(name);

    rOut.WriteULong(static_cast<uint32>(LayerOffsets.size()));

    for (const uint32 offset : LayerOffsets)
        rOut.WriteULong(offset);

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
    const uint32 kSizeThreshold = 0x20000;
    mSectionData.WriteToBoundary(32, 0);

    const uint32 SecSize = mSectionData.Size();
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
    else
    {
        AddSectionToBlock();
    }

    mSectionData.Clear();
}

void CAreaCooker::FinishBlock()
{
    if (mCurBlock.NumSections == 0) return;

    std::vector<uint8> CompressedBuf(mCompressedData.Size() * 2);
    const bool EnableCompression = (mVersion >= EGame::Echoes) && mpArea->mUsesCompression && !gkForceDisableCompression;
    const bool UseZlib = (mVersion == EGame::DKCReturns);

    uint32 CompressedSize = 0;
    bool WriteCompressedData = false;

    if (EnableCompression)
    {
        const bool Success = CompressionUtil::CompressSegmentedData(static_cast<uint8*>(mCompressedData.Data()), mCompressedData.Size(), CompressedBuf.data(), CompressedSize, UseZlib, true);
        const uint32 PadBytes = (32 - (CompressedSize % 32)) & 0x1F;
        WriteCompressedData = Success && (CompressedSize + PadBytes < static_cast<uint32>(mCompressedData.Size()));
    }

    if (WriteCompressedData)
    {
        uint32 PadBytes = 32 - (CompressedSize % 32);
        PadBytes &= 0x1F;

        for (uint32 iPad = 0; iPad < PadBytes; iPad++)
            mAreaData.WriteUByte(0);

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
    for (uint32 iSec = 0; iSec < Cooker.mSCLYSecNum; iSec++)
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
    const uint32 PostSCLY = (Cooker.mVersion <= EGame::Prime ? Cooker.mSCLYSecNum + 1 : Cooker.mSCGNSecNum + 1);
    for (size_t iSec = PostSCLY; iSec < pArea->mSectionDataBuffers.size(); iSec++)
    {
        if (iSec == Cooker.mModulesSecNum)
        {
            Cooker.WriteModules(Cooker.mSectionData);
        }
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

uint32 CAreaCooker::GetMREAVersion(EGame Version)
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
