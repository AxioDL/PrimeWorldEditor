#include "CAreaLoader.h"
#include "CCollisionLoader.h"
#include "CModelLoader.h"
#include "CMaterialLoader.h"
#include "CScriptLoader.h"
#include <Common/Log.h>

#include <Common/CFourCC.h>
#include <Common/CompressionUtil.h>

#include <iostream>

CAreaLoader::CAreaLoader()
{
    mpMREA = nullptr;
    mHasDecompressedBuffer = false;
    mGeometryBlockNum      = -1;
    mScriptLayerBlockNum   = -1;
    mCollisionBlockNum     = -1;
    mUnknownBlockNum       = -1;
    mLightsBlockNum        = -1;
    mEmptyBlockNum         = -1;
    mPathBlockNum          = -1;
    mOctreeBlockNum        = -1;
    mScriptGeneratorBlockNum          = -1;
    mFFFFBlockNum          = -1;
    mUnknown2BlockNum      = -1;
    mEGMCBlockNum          = -1;
    mBoundingBoxesBlockNum = -1;
    mDependenciesBlockNum  = -1;
    mGPUBlockNum           = -1;
    mPVSBlockNum           = -1;
    mRSOBlockNum           = -1;
}

CAreaLoader::~CAreaLoader()
{
    if (mHasDecompressedBuffer)
    {
        delete mpMREA;
        delete[] mDecmpBuffer;
    }
}

// ************ PRIME ************
void CAreaLoader::ReadHeaderPrime()
{
    Log::FileWrite(mpMREA->GetSourceString(), "Reading MREA header (MP1)");
    mpArea->mTransform = CTransform4f(*mpMREA);
    mNumMeshes = mpMREA->ReadLong();
    u32 mNumBlocks = mpMREA->ReadLong();

    mGeometryBlockNum = mpMREA->ReadLong();
    mScriptLayerBlockNum = mpMREA->ReadLong();
    mCollisionBlockNum = mpMREA->ReadLong();
    mUnknownBlockNum = mpMREA->ReadLong();
    mLightsBlockNum = mpMREA->ReadLong();
    mEmptyBlockNum = mpMREA->ReadLong();
    mPathBlockNum = mpMREA->ReadLong();
    mOctreeBlockNum = mpMREA->ReadLong();

    mpSectionMgr = new CSectionMgrIn(mNumBlocks, mpMREA);
    mpMREA->SeekToBoundary(32);
    mpSectionMgr->Init();
    LoadSectionDataBuffers();

    mpArea->mOriginalWorldMeshCount = mNumMeshes;
}

void CAreaLoader::ReadGeometryPrime()
{
    Log::FileWrite(mpMREA->GetSourceString(), "Reading MREA world geometry (MP1/MP2)");
    mpSectionMgr->ToSection(mGeometryBlockNum);

    // Materials
    mpArea->mMaterialSet = CMaterialLoader::LoadMaterialSet(*mpMREA, mVersion);
    mpSectionMgr->ToNextSection();

    // Geometry
    std::vector<CModel*> FileModels;

    for (u32 iMesh = 0; iMesh < mNumMeshes; iMesh++)
    {
        CModel *pModel = CModelLoader::LoadWorldModel(*mpMREA, *mpSectionMgr, *mpArea->mMaterialSet, mVersion);
        FileModels.push_back(pModel);

        if (mVersion <= ePrime)
            mpArea->AddWorldModel(pModel);

        // For Echoes+, load surface mesh IDs, then skip to the start of the next mesh
        else
        {
            u16 NumSurfaces = mpMREA->ReadShort();

            for (u32 iSurf = 0; iSurf < NumSurfaces; iSurf++)
            {
                mpMREA->Seek(0x2, SEEK_CUR);
                pModel->GetSurface(iSurf)->MeshID = mpMREA->ReadShort();
            }

            mpSectionMgr->ToNextSection();
            mpSectionMgr->ToNextSection();
        }
    }

    // Split meshes
    if (mVersion >= eEchoesDemo)
    {
        std::vector<CModel*> SplitModels;
        CModelLoader::BuildWorldMeshes(FileModels, SplitModels, true);

        for (u32 iMdl = 0; iMdl < SplitModels.size(); iMdl++)
            mpArea->AddWorldModel(SplitModels[iMdl]);
    }

    mpArea->MergeTerrain();
}

void CAreaLoader::ReadSCLYPrime()
{
    // Prime, Echoes Demo
    Log::FileWrite(mpMREA->GetSourceString(), "Reading MREA script layers (MP1)");
    mpSectionMgr->ToSection(mScriptLayerBlockNum);

    CFourCC SCLY(*mpMREA);
    if (SCLY != "SCLY")
    {
        Log::FileError(mpMREA->GetSourceString(), mpMREA->Tell() - 4, "Invalid SCLY magic: " + SCLY.ToString());
        return;
    }
    mpMREA->Seek(0x4, SEEK_CUR); // Skipping unknown value which is always 4

    // Read layer sizes
    mNumLayers = mpMREA->ReadLong();
    mpArea->mScriptLayers.resize(mNumLayers);
    std::vector<u32> LayerSizes(mNumLayers);

    for (u32 iLyr = 0; iLyr < mNumLayers; iLyr++)
        LayerSizes[iLyr] = mpMREA->ReadLong();

    // SCLY
    for (u32 iLyr = 0; iLyr < mNumLayers; iLyr++)
    {
        u32 Next = mpMREA->Tell() + LayerSizes[iLyr];
        mpArea->mScriptLayers[iLyr] = CScriptLoader::LoadLayer(*mpMREA, mpArea, mVersion);
        mpMREA->Seek(Next, SEEK_SET);
    }

    // SCGN
    if (mVersion == eEchoesDemo)
    {
        mpSectionMgr->ToSection(mScriptGeneratorBlockNum);

        CFourCC SCGN(*mpMREA);
        if (SCGN != "SCGN")
            Log::FileError(mpMREA->GetSourceString(), mpMREA->Tell() - 4, "Invalid SCGN magic: " + SCGN.ToString());

        else
        {
            mpMREA->Seek(0x1, SEEK_CUR);
            CScriptLayer *pLayer = CScriptLoader::LoadLayer(*mpMREA, mpArea, mVersion);

            if (pLayer)
            {
                mpArea->mpGeneratorLayer = pLayer;
                pLayer->SetName("Generated Objects");
                pLayer->SetActive(true);
            }
        }
    }

    SetUpObjects();
}

void CAreaLoader::ReadLightsPrime()
{
    Log::FileWrite(mpMREA->GetSourceString(), "Reading MREA dynamic lights (MP1/MP2)");
    mpSectionMgr->ToSection(mLightsBlockNum);

    u32 babedead = mpMREA->ReadLong();
    if (babedead != 0xbabedead) return;

    mpArea->mLightLayers.resize(2);

    for (u32 ly = 0; ly < 2; ly++)
    {
        u32 NumLights = mpMREA->ReadLong();
        mpArea->mLightLayers[ly].resize(NumLights);

        for (u32 l = 0; l < NumLights; l++)
        {
            ELightType Type = ELightType(mpMREA->ReadLong());
            CVector3f Color(*mpMREA);
            CVector3f Position(*mpMREA);
            CVector3f Direction(*mpMREA);
            float Multiplier = mpMREA->ReadFloat();
            float SpotCutoff = mpMREA->ReadFloat();
            mpMREA->Seek(0x9, SEEK_CUR);
            u32 FalloffType = mpMREA->ReadLong();
            mpMREA->Seek(0x4, SEEK_CUR);

            // Relevant data is read - now we process and form a CLight out of it
            CLight *Light;

            CColor LightColor = CColor(Color.x, Color.y, Color.z, 0.f);
            if (Multiplier < FLT_EPSILON)
                Multiplier = FLT_EPSILON;

            // Local Ambient
            if (Type == eLocalAmbient)
            {
                Color *= Multiplier;

                // Clamp
                if (Color.x > 1.f) Color.x = 1.f;
                if (Color.y > 1.f) Color.y = 1.f;
                if (Color.z > 1.f) Color.z = 1.f;
                CColor MultColor(Color.x, Color.y, Color.z, 1.f);

                Light = CLight::BuildLocalAmbient(Position, MultColor);
            }

            // Directional
            else if (Type == eDirectional)
            {
                Light = CLight::BuildDirectional(Position, Direction, LightColor);
            }

            // Spot
            else if (Type == eSpot)
            {
                Light = CLight::BuildSpot(Position, Direction.Normalized(), LightColor, SpotCutoff);

                float DistAttenA = (FalloffType == 0) ? (2.f / Multiplier) : 0.f;
                float DistAttenB = (FalloffType == 1) ? (250.f / Multiplier) : 0.f;
                float DistAttenC = (FalloffType == 2) ? (25000.f / Multiplier) : 0.f;
                Light->SetDistAtten(DistAttenA, DistAttenB, DistAttenC);
            }

            // Custom
            else
            {
                float DistAttenA = (FalloffType == 0) ? (2.f / Multiplier) : 0.f;
                float DistAttenB = (FalloffType == 1) ? (249.9998f / Multiplier) : 0.f;
                float DistAttenC = (FalloffType == 2) ? (25000.f / Multiplier) : 0.f;

                Light = CLight::BuildCustom(Position, Direction, LightColor,
                                            DistAttenA, DistAttenB, DistAttenC,
                                            1.f, 0.f, 0.f);
            }

            Light->SetLayer(ly);
            mpArea->mLightLayers[ly][l] = Light;
        }
    }
}

// ************ ECHOES ************
void CAreaLoader::ReadHeaderEchoes()
{
    // This function reads the header for Echoes and the Echoes demo disc
    Log::FileWrite(mpMREA->GetSourceString(), "Reading MREA header (MP2)");
    mpArea->mTransform = CTransform4f(*mpMREA);
    mNumMeshes = mpMREA->ReadLong();
    if (mVersion == eEchoes) mNumLayers = mpMREA->ReadLong();
    u32 numBlocks = mpMREA->ReadLong();

    mGeometryBlockNum = mpMREA->ReadLong();
    mScriptLayerBlockNum = mpMREA->ReadLong();
    mScriptGeneratorBlockNum = mpMREA->ReadLong();
    mCollisionBlockNum = mpMREA->ReadLong();
    mUnknownBlockNum = mpMREA->ReadLong();
    mLightsBlockNum = mpMREA->ReadLong();
    mEmptyBlockNum = mpMREA->ReadLong();
    mPathBlockNum = mpMREA->ReadLong();
    mFFFFBlockNum = mpMREA->ReadLong();
    mUnknown2BlockNum = mpMREA->ReadLong();
    mEGMCBlockNum = mpMREA->ReadLong();
    if (mVersion == eEchoes) mClusters.resize(mpMREA->ReadLong());
    mpMREA->SeekToBoundary(32);

    mpSectionMgr = new CSectionMgrIn(numBlocks, mpMREA);
    mpMREA->SeekToBoundary(32);

    if (mVersion == eEchoes)
    {
        ReadCompressedBlocks();
        Decompress();
    }

    mpSectionMgr->Init();
    LoadSectionDataBuffers();

    mpArea->mOriginalWorldMeshCount = mNumMeshes;
}

void CAreaLoader::ReadSCLYEchoes()
{
    // MP2, MP3 Proto, MP3, DKCR
    Log::FileWrite(mpMREA->GetSourceString(), "Reading MREA script layers (MP2/MP3/DKCR)");
    mpSectionMgr->ToSection(mScriptLayerBlockNum);
    mpArea->mScriptLayers.resize(mNumLayers);

    // SCLY
    for (u32 iLyr = 0; iLyr < mNumLayers; iLyr++)
    {
        CFourCC SCLY(*mpMREA);
        if (SCLY != "SCLY")
        {
            Log::FileError(mpMREA->GetSourceString(), mpMREA->Tell() - 4, "Layer " + TString::FromInt32(iLyr, 0, 10) + " - Invalid SCLY magic: " + SCLY.ToString());
            mpSectionMgr->ToNextSection();
            continue;
        }

        mpMREA->Seek(0x5, SEEK_CUR); // Skipping unknown + layer index
        mpArea->mScriptLayers[iLyr] = CScriptLoader::LoadLayer(*mpMREA, mpArea, mVersion);
        mpSectionMgr->ToNextSection();
    }

    // SCGN
    CFourCC SCGN(*mpMREA);
    if (SCGN != "SCGN")
    {
        Log::FileError(mpMREA->GetSourceString(), mpMREA->Tell() - 4, "Invalid SCGN magic: " + SCGN.ToString());
        return;
    }

    mpMREA->Seek(0x1, SEEK_CUR); // Skipping unknown
    mpArea->mpGeneratorLayer = CScriptLoader::LoadLayer(*mpMREA, mpArea, mVersion);

    if (mpArea->mpGeneratorLayer)
    {
        mpArea->mpGeneratorLayer->SetName("Generated Objects");
        mpArea->mpGeneratorLayer->SetActive(true);
    }

    SetUpObjects();
}

// ************ CORRUPTION ************
void CAreaLoader::ReadHeaderCorruption()
{
    // This function reads the header for MP3, the MP3 prototype, and DKCR
    Log::FileWrite(mpMREA->GetSourceString(), "Reading MREA header (MP3/DKCR)");
    mpArea->mTransform = CTransform4f(*mpMREA);
    mNumMeshes = mpMREA->ReadLong();
    mNumLayers = mpMREA->ReadLong();
    u32 NumSections = mpMREA->ReadLong();
    mClusters.resize(mpMREA->ReadLong());
    u32 SectionNumberCount = mpMREA->ReadLong();
    mpMREA->SeekToBoundary(32);

    mpSectionMgr = new CSectionMgrIn(NumSections, mpMREA);
    mpMREA->SeekToBoundary(32);

    ReadCompressedBlocks();

    for (u32 iNum = 0; iNum < SectionNumberCount; iNum++)
    {
        CFourCC Type(*mpMREA);
        u32 Num = mpMREA->ReadLong();

        if      (Type == "AABB") mBoundingBoxesBlockNum = Num;
        else if (Type == "COLI") mCollisionBlockNum = Num;
        else if (Type == "DEPS") mDependenciesBlockNum = Num;
        else if (Type == "EGMC") mEGMCBlockNum = Num;
        else if (Type == "GPUD") mGPUBlockNum = Num;
        else if (Type == "LITE") mLightsBlockNum = Num;
        else if (Type == "PFL2") mPathBlockNum = Num;
        else if (Type == "PVS!") mPVSBlockNum = Num;
        else if (Type == "ROCT") mOctreeBlockNum = Num;
        else if (Type == "RSOS") mRSOBlockNum = Num;
        else if (Type == "SOBJ") mScriptLayerBlockNum = Num;
        else if (Type == "SGEN") mScriptGeneratorBlockNum = Num;
        else if (Type == "WOBJ") mGeometryBlockNum = Num; // note WOBJ can show up multiple times, but is always 0

        CGameArea::SSectionNumber SecNum;
        SecNum.SectionID = Type;
        SecNum.Index = Num;
        mpArea->mSectionNumbers.push_back(SecNum);
    }

    mpMREA->SeekToBoundary(32);
    Decompress();
    mpSectionMgr->Init();
    LoadSectionDataBuffers();

    mpArea->mOriginalWorldMeshCount = mNumMeshes;
}

void CAreaLoader::ReadGeometryCorruption()
{
    Log::FileWrite(mpMREA->GetSourceString(), "Reading MREA world geometry (MP3)");
    mpSectionMgr->ToSection(mGeometryBlockNum);

    // Materials
    mpArea->mMaterialSet = CMaterialLoader::LoadMaterialSet(*mpMREA, mVersion);
    mpSectionMgr->ToNextSection();

    // Geometry
    std::vector<CModel*> FileModels;
    u32 CurWOBJSection = 1;
    u32 CurGPUSection = mGPUBlockNum;

    for (u32 iMesh = 0; iMesh < mNumMeshes; iMesh++)
    {
        CModel *pWorldModel = CModelLoader::LoadCorruptionWorldModel(*mpMREA, *mpSectionMgr, *mpArea->mMaterialSet, CurWOBJSection, CurGPUSection, mVersion);
        FileModels.push_back(pWorldModel);

        CurWOBJSection += 4;
        CurGPUSection = mpSectionMgr->CurrentSection();

        // Load surface mesh IDs
        mpSectionMgr->ToSection(CurWOBJSection -  2);
        u16 NumSurfaces = mpMREA->ReadShort();

        for (u32 iSurf = 0; iSurf < NumSurfaces; iSurf++)
        {
            mpMREA->Seek(0x2, SEEK_CUR);
            pWorldModel->GetSurface(iSurf)->MeshID = mpMREA->ReadShort();
        }
    }

    std::vector<CModel*> SplitModels;
    CModelLoader::BuildWorldMeshes(FileModels, SplitModels, true);

    for (u32 iMdl = 0; iMdl < SplitModels.size(); iMdl++)
        mpArea->AddWorldModel(SplitModels[iMdl]);

    mpArea->MergeTerrain();
}

void CAreaLoader::ReadLightsCorruption()
{
    Log::FileWrite(mpMREA->GetSourceString(), "Reading MREA dynamic lights (MP3)");
    mpSectionMgr->ToSection(mLightsBlockNum);

    u32 babedead = mpMREA->ReadLong();
    if (babedead != 0xbabedead) return;

    mpArea->mLightLayers.resize(4);

    for (u32 iLayer = 0; iLayer < 4; iLayer++)
    {
        u32 NumLights = mpMREA->ReadLong();
        mpArea->mLightLayers[iLayer].resize(NumLights);

        for (u32 iLight = 0; iLight < NumLights; iLight++)
        {
            ELightType Type = (ELightType) mpMREA->ReadLong();

            float r = mpMREA->ReadFloat();
            float g = mpMREA->ReadFloat();
            float b = mpMREA->ReadFloat();
            float a = mpMREA->ReadFloat();
            CColor LightColor(r, g, b, a);

            CVector3f Position(*mpMREA);
            CVector3f Direction(*mpMREA);
            mpMREA->Seek(0xC, SEEK_CUR);

            float Multiplier = mpMREA->ReadFloat();
            float SpotCutoff = mpMREA->ReadFloat();
            mpMREA->Seek(0x9, SEEK_CUR);
            u32 FalloffType = mpMREA->ReadLong();
            mpMREA->Seek(0x18, SEEK_CUR);

            // Relevant data is read - now we process and form a CLight out of it
            CLight *Light;

            if (Multiplier < FLT_EPSILON)
                Multiplier = FLT_EPSILON;

            // Local Ambient
            if (Type == eLocalAmbient)
            {
                Light = CLight::BuildLocalAmbient(Position, LightColor * Multiplier);
            }

            // Directional
            else if (Type == eDirectional)
            {
                Light = CLight::BuildDirectional(Position, Direction, LightColor);
            }

            // Spot
            else if (Type == eSpot)
            {
                Light = CLight::BuildSpot(Position, Direction.Normalized(), LightColor, SpotCutoff);

                float DistAttenA = (FalloffType == 0) ? (2.f / Multiplier) : 0.f;
                float DistAttenB = (FalloffType == 1) ? (250.f / Multiplier) : 0.f;
                float DistAttenC = (FalloffType == 2) ? (25000.f / Multiplier) : 0.f;
                Light->SetDistAtten(DistAttenA, DistAttenB, DistAttenC);
            }

            // Custom
            else
            {
                float DistAttenA = (FalloffType == 0) ? (2.f / Multiplier) : 0.f;
                float DistAttenB = (FalloffType == 1) ? (249.9998f / Multiplier) : 0.f;
                float DistAttenC = (FalloffType == 2) ? (25000.f / Multiplier) : 0.f;

                Light = CLight::BuildCustom(Position, Direction, LightColor,
                                            DistAttenA, DistAttenB, DistAttenC,
                                            1.f, 0.f, 0.f);
            }

            Light->SetLayer(iLayer);
            mpArea->mLightLayers[iLayer][iLight] = Light;
        }
    }
}

// ************ COMMON ************
void CAreaLoader::ReadCompressedBlocks()
{
    mTotalDecmpSize = 0;

    for (u32 c = 0; c < mClusters.size(); c++)
    {
        mClusters[c].BufferSize = mpMREA->ReadLong();
        mClusters[c].DecompressedSize = mpMREA->ReadLong();
        mClusters[c].CompressedSize = mpMREA->ReadLong();
        mClusters[c].NumSections = mpMREA->ReadLong();
        mTotalDecmpSize += mClusters[c].DecompressedSize;

        if (mClusters[c].CompressedSize != 0) mpArea->mUsesCompression = true;
    }

    mpMREA->SeekToBoundary(32);
}

void CAreaLoader::Decompress()
{
    // This function decompresses compressed clusters into a buffer.
    // It should be called at the beginning of the first compressed cluster.
    Log::FileWrite(mpMREA->GetSourceString(), "Decompressing MREA data");
    if (mVersion < eEchoes) return;

    // Decompress clusters
    mDecmpBuffer = new u8[mTotalDecmpSize];
    u32 Offset = 0;

    for (u32 c = 0; c < mClusters.size(); c++)
    {
        SCompressedCluster *cc = &mClusters[c];

        // Is it decompressed already?
        if (mClusters[c].CompressedSize == 0)
        {
            mpMREA->ReadBytes(mDecmpBuffer + Offset, cc->DecompressedSize);
            Offset += cc->DecompressedSize;
        }

        else
        {
            u32 StartOffset = 32 - (mClusters[c].CompressedSize % 32); // For some reason they pad the beginning instead of the end
            if (StartOffset != 32)
                mpMREA->Seek(StartOffset, SEEK_CUR);

            std::vector<u8> cmp(mClusters[c].CompressedSize);
            mpMREA->ReadBytes(cmp.data(), cmp.size());

            bool Success = CompressionUtil::DecompressSegmentedData(cmp.data(), cmp.size(), mDecmpBuffer + Offset, cc->DecompressedSize);
            if (!Success)
                throw "Failed to decompress MREA!";

            Offset += cc->DecompressedSize;
        }
    }

    TString Source = mpMREA->GetSourceString();
    mpMREA = new CMemoryInStream(mDecmpBuffer, mTotalDecmpSize, IOUtil::eBigEndian);
    mpMREA->SetSourceString(Source.ToStdString());
    mpSectionMgr->SetInputStream(mpMREA);
    mHasDecompressedBuffer = true;
}

void CAreaLoader::LoadSectionDataBuffers()
{
   mpArea->mSectionDataBuffers.resize(mpSectionMgr->NumSections());
   mpSectionMgr->ToSection(0);

   for (u32 iSec = 0; iSec < mpSectionMgr->NumSections(); iSec++)
   {
       u32 Size = mpSectionMgr->CurrentSectionSize();
       mpArea->mSectionDataBuffers[iSec].resize(Size);
       mpMREA->ReadBytes(mpArea->mSectionDataBuffers[iSec].data(), mpArea->mSectionDataBuffers[iSec].size());
       mpSectionMgr->ToNextSection();
   }
}

void CAreaLoader::ReadCollision()
{
    Log::FileWrite(mpMREA->GetSourceString(), "Reading collision (MP1/MP2/MP3)");
    mpSectionMgr->ToSection(mCollisionBlockNum);
    mpArea->mCollision = CCollisionLoader::LoadAreaCollision(*mpMREA);
}

void CAreaLoader::ReadEGMC()
{
    Log::FileWrite(mpMREA->GetSourceString(), "Reading EGMC");
    mpSectionMgr->ToSection(mEGMCBlockNum);
    CUniqueID EGMC(*mpMREA, (mVersion <= eEchoes ? e32Bit : e64Bit));
    mpArea->mpPoiToWorldMap = gResCache.GetResource(EGMC, "EGMC");
}

void CAreaLoader::SetUpObjects()
{
    // Iterate over all objects
    for (u32 iLyr = 0; iLyr < mpArea->GetScriptLayerCount() + 1; iLyr++)
    {
        CScriptLayer *pLayer;
        if (iLyr < mpArea->GetScriptLayerCount()) pLayer = mpArea->mScriptLayers[iLyr];

        else
        {
            pLayer = mpArea->GetGeneratorLayer();
            if (!pLayer) break;
        }

        for (u32 iObj = 0; iObj < pLayer->NumInstances(); iObj++)
        {
            // Add object to object map
            CScriptObject *pObj = (*pLayer)[iObj];
            mpArea->mObjectMap[pObj->InstanceID()] = pObj;

            // Store outgoing connections
            for (u32 iCon = 0; iCon < pObj->NumOutLinks(); iCon++)
            {
                SLink Connection = pObj->OutLink(iCon);

                SLink NewConnection;
                NewConnection.State = Connection.State;
                NewConnection.Message = Connection.Message;
                NewConnection.ObjectID = pObj->InstanceID();
                mConnectionMap[Connection.ObjectID].push_back(NewConnection);
            }
        }
    }

    // Store connections
    for (auto it = mpArea->mObjectMap.begin(); it != mpArea->mObjectMap.end(); it++)
    {
        u32 InstanceID = it->first;
        auto iConMap = mConnectionMap.find(InstanceID);

        if (iConMap != mConnectionMap.end())
        {
            CScriptObject *pObj = mpArea->GetInstanceByID(InstanceID);
            pObj->mInConnections = iConMap->second;
        }
    }
}

// ************ STATIC ************
CGameArea* CAreaLoader::LoadMREA(IInputStream& MREA)
{
    CAreaLoader Loader;

    // Validation
    if (!MREA.IsValid()) return nullptr;

    u32 deadbeef = MREA.ReadLong();
    if (deadbeef != 0xdeadbeef)
    {
        Log::FileError(MREA.GetSourceString(), "Invalid MREA magic: " + TString::HexString(deadbeef));
        return nullptr;
    }

    // Header
    Loader.mpArea = new CGameArea;
    u32 version = MREA.ReadLong();
    Loader.mVersion = GetFormatVersion(version);
    Loader.mpArea->mVersion = Loader.mVersion;
    Loader.mpMREA = &MREA;

    switch (Loader.mVersion)
    {
        case ePrimeDemo:
        case ePrime:
            Loader.ReadHeaderPrime();
            Loader.ReadGeometryPrime();
            Loader.ReadSCLYPrime();
            Loader.ReadCollision();
            Loader.ReadLightsPrime();
            break;
        case eEchoesDemo:
            Loader.ReadHeaderEchoes();
            Loader.ReadGeometryPrime();
            Loader.ReadSCLYPrime();
            Loader.ReadCollision();
            Loader.ReadLightsPrime();
            Loader.ReadEGMC();
            break;
        case eEchoes:
            Loader.ReadHeaderEchoes();
            Loader.ReadGeometryPrime();
            Loader.ReadSCLYEchoes();
            Loader.ReadCollision();
            Loader.ReadLightsPrime();
            Loader.ReadEGMC();
            break;
        case eCorruptionProto:
            Loader.ReadHeaderCorruption();
            Loader.ReadGeometryPrime();
            Loader.ReadSCLYEchoes();
            Loader.ReadCollision();
            Loader.ReadLightsCorruption();
            Loader.ReadEGMC();
            break;
        case eCorruption:
        case eReturns:
            Loader.ReadHeaderCorruption();
            Loader.ReadGeometryCorruption();
            Loader.ReadSCLYEchoes();
            Loader.ReadCollision();
            if (Loader.mVersion == eCorruption)
            {
                Loader.ReadLightsCorruption();
                Loader.ReadEGMC();
            }
            break;
        default:
            Log::FileError(MREA.GetSourceString(), "Unsupported MREA version: " + TString::HexString(version));
            Loader.mpArea.Delete();
            return nullptr;
    }

    // Cleanup
    delete Loader.mpSectionMgr;
    return Loader.mpArea;
}

EGame CAreaLoader::GetFormatVersion(u32 version)
{
    switch (version)
    {
        case 0xC: return ePrimeDemo;
        case 0xF: return ePrime;
        case 0x15: return eEchoesDemo;
        case 0x19: return eEchoes;
        case 0x1D: return eCorruptionProto;
        case 0x1E: return eCorruption;
        case 0x20: return eReturns;
        default: return eUnknownVersion;
    }
}
