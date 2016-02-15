#include "CAreaCooker.h"
#include "CScriptCooker.h"
#include <Common/Log.h>

CAreaCooker::CAreaCooker()
{
}

void CAreaCooker::DetermineSectionNumbers()
{
    mGeometrySecNum = 0;

    // Determine how many sections are taken up by geometry...
    u32 GeometrySections = 1; // Starting at 1 to account for materials

    // Each world mesh has (7 + surface count) sections
    // header, verts, normals, colors, float UVs, short UVs, surface offsets + surfaces
    for (u32 iMesh = 0; iMesh < mpArea->mTerrainModels.size(); iMesh++)
    {
        CModel *pModel = mpArea->mTerrainModels[iMesh];
        GeometrySections += (7 + pModel->GetSurfaceCount());
    }

    // Set section numbers
    mArotSecNum = mGeometrySecNum + GeometrySections;
    mSclySecNum = mArotSecNum + 1;
    mCollisionSecNum = mSclySecNum + 1;
    mUnknownSecNum = mCollisionSecNum + 1;
    mLightsSecNum = mUnknownSecNum + 1;
    mVisiSecNum = mLightsSecNum + 1;
    mPathSecNum = mVisiSecNum + 1;
}

void CAreaCooker::WritePrimeHeader(IOutputStream& rOut)
{
    rOut.WriteLong(0xDEADBEEF);
    rOut.WriteLong(GetMREAVersion(mVersion));
    mpArea->mTransform.Write(rOut);
    rOut.WriteLong(mpArea->mTerrainModels.size());
    rOut.WriteLong(mpArea->mSectionDataBuffers.size());

    rOut.WriteLong(mGeometrySecNum);
    rOut.WriteLong(mSclySecNum);
    rOut.WriteLong(mCollisionSecNum);
    rOut.WriteLong(mUnknownSecNum);
    rOut.WriteLong(mLightsSecNum);
    rOut.WriteLong(mVisiSecNum);
    rOut.WriteLong(mPathSecNum);
    rOut.WriteLong(mArotSecNum);

    mSectionSizesOffset = rOut.Tell();
    for (u32 iSec = 0; iSec < mpArea->mSectionDataBuffers.size(); iSec++)
        rOut.WriteLong(0);

    rOut.WriteToBoundary(32, 0);

    mSectionMgr.SetSectionCount(mpArea->mSectionDataBuffers.size());
    mSectionMgr.Init(rOut);
}

void CAreaCooker::WritePrimeSCLY(IOutputStream& rOut)
{
    u32 NumLayers = mpArea->mScriptLayers.size();
    rOut.WriteString("SCLY", 4);
    rOut.WriteLong(0x1); // Unknown value, but it's always 1
    rOut.WriteLong(NumLayers);

    u32 LayerSizesStart = rOut.Tell();
    for (u32 iLyr = 0; iLyr < NumLayers; iLyr++)
        rOut.WriteLong(0);

    std::vector<u32> LayerSizes(NumLayers);

    for (u32 iLyr = 0; iLyr < mpArea->mScriptLayers.size(); iLyr++)
    {
        u32 LayerStart = rOut.Tell();
        CScriptCooker::WriteLayer(mpArea->Version(), mpArea->mScriptLayers[iLyr], rOut);
        LayerSizes[iLyr] = rOut.Tell() - LayerStart;
    }

    u32 LayersEnd = rOut.Tell();
    rOut.Seek(LayerSizesStart, SEEK_SET);

    for (u32 iLyr = 0; iLyr < NumLayers; iLyr++)
        rOut.WriteLong(LayerSizes[iLyr]);

    rOut.Seek(LayersEnd, SEEK_SET);
    rOut.WriteToBoundary(32, 0);
}

// ************ STATIC ************
void CAreaCooker::WriteCookedArea(CGameArea *pArea, IOutputStream& rOut)
{
    CAreaCooker Cooker;
    Cooker.mpArea = pArea;
    Cooker.mVersion = pArea->Version();

    if (Cooker.mVersion > ePrime)
    {
        Log::Error("Area cooking is not supported for games other than Metroid Prime");
        return;
    }

    // Write header
    Cooker.DetermineSectionNumbers();
    Cooker.WritePrimeHeader(rOut);

    // Write pre-SCLY data sections
    for (u32 iSec = 0; iSec < Cooker.mSclySecNum; iSec++)
    {
        rOut.WriteBytes(pArea->mSectionDataBuffers[iSec].data(), pArea->mSectionDataBuffers[iSec].size());
        Cooker.mSectionMgr.AddSize(rOut);
    }

    // Write SCLY
    Cooker.WritePrimeSCLY(rOut);
    Cooker.mSectionMgr.AddSize(rOut);

    // Write post-SCLY data sections
    for (u32 iSec = Cooker.mSclySecNum + 1; iSec < pArea->mSectionDataBuffers.size(); iSec++)
    {
        rOut.WriteBytes(pArea->mSectionDataBuffers[iSec].data(), pArea->mSectionDataBuffers[iSec].size());
        Cooker.mSectionMgr.AddSize(rOut);
    }

    // Write section sizes
    u32 AreaEnd = rOut.Tell();
    rOut.Seek(Cooker.mSectionSizesOffset, SEEK_SET);
    Cooker.mSectionMgr.WriteSizes(rOut);
    rOut.Seek(AreaEnd, SEEK_SET);
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
