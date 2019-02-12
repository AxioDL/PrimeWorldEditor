#include "CAreaLoader.h"
#include "CCollisionLoader.h"
#include "CModelLoader.h"
#include "CMaterialLoader.h"
#include "CScriptLoader.h"
#include "Core/CompressionUtil.h"
#include <Common/Log.h>

#include <Common/CFourCC.h>

#include <iostream>

CAreaLoader::CAreaLoader()
    : mpMREA(nullptr)
    , mHasDecompressedBuffer(false)
    , mGeometryBlockNum(-1)
    , mScriptLayerBlockNum(-1)
    , mCollisionBlockNum(-1)
    , mUnknownBlockNum(-1)
    , mLightsBlockNum(-1)
    , mVisiBlockNum(-1)
    , mPathBlockNum(-1)
    , mOctreeBlockNum(-1)
    , mScriptGeneratorBlockNum(-1)
    , mFFFFBlockNum(-1)
    , mPTLABlockNum(-1)
    , mEGMCBlockNum(-1)
    , mBoundingBoxesBlockNum(-1)
    , mDependenciesBlockNum(-1)
    , mGPUBlockNum(-1)
    , mRSOBlockNum(-1)
{
}

CAreaLoader::~CAreaLoader()
{
    if (mHasDecompressedBuffer)
    {
        delete mpMREA;
        delete[] mpDecmpBuffer;
    }
}

// ************ PRIME ************
void CAreaLoader::ReadHeaderPrime()
{
    mpArea->mTransform = CTransform4f(*mpMREA);
    mNumMeshes = mpMREA->ReadLong();
    uint32 mNumBlocks = mpMREA->ReadLong();

    mGeometryBlockNum = mpMREA->ReadLong();
    mScriptLayerBlockNum = mpMREA->ReadLong();
    mCollisionBlockNum = mpMREA->ReadLong();
    mUnknownBlockNum = mpMREA->ReadLong();
    mLightsBlockNum = mpMREA->ReadLong();
    mVisiBlockNum = mpMREA->ReadLong();
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
    mpSectionMgr->ToSection(mGeometryBlockNum);

    // Materials
    mpArea->mpMaterialSet = CMaterialLoader::LoadMaterialSet(*mpMREA, mVersion);
    mpSectionMgr->ToNextSection();

    // Geometry
    std::vector<CModel*> FileModels;

    for (uint32 iMesh = 0; iMesh < mNumMeshes; iMesh++)
    {
        CModel *pModel = CModelLoader::LoadWorldModel(*mpMREA, *mpSectionMgr, *mpArea->mpMaterialSet, mVersion);
        FileModels.push_back(pModel);

        if (mVersion <= EGame::Prime)
            mpArea->AddWorldModel(pModel);

        // For Echoes+, load surface mesh IDs, then skip to the start of the next mesh
        else
        {
            uint16 NumSurfaces = mpMREA->ReadShort();

            for (uint32 iSurf = 0; iSurf < NumSurfaces; iSurf++)
            {
                mpMREA->Seek(0x2, SEEK_CUR);
                pModel->GetSurface(iSurf)->MeshID = mpMREA->ReadShort();
            }

            mpSectionMgr->ToNextSection();
            mpSectionMgr->ToNextSection();
        }
    }

    // Split meshes
    if (mVersion >= EGame::EchoesDemo)
    {
        std::vector<CModel*> SplitModels;
        CModelLoader::BuildWorldMeshes(FileModels, SplitModels, true);

        for (uint32 iMdl = 0; iMdl < SplitModels.size(); iMdl++)
            mpArea->AddWorldModel(SplitModels[iMdl]);
    }

    mpArea->MergeTerrain();
}

void CAreaLoader::ReadSCLYPrime()
{
    // Prime, Echoes Demo
    mpSectionMgr->ToSection(mScriptLayerBlockNum);

    CFourCC SCLY(*mpMREA);
    if (SCLY != FOURCC('SCLY'))
    {
        errorf("%s [0x%X]: Invalid SCLY magic: %s", *mpMREA->GetSourceString(), mpMREA->Tell() - 4, *SCLY.ToString());
        return;
    }
    mpMREA->Seek(mVersion <= EGame::Prime ? 4 : 1, SEEK_CUR); // Skipping unknown value which is always 1

    // Read layer sizes
    mNumLayers = mpMREA->ReadLong();
    mpArea->mScriptLayers.resize(mNumLayers);
    std::vector<uint32> LayerSizes(mNumLayers);

    for (uint32 iLyr = 0; iLyr < mNumLayers; iLyr++)
        LayerSizes[iLyr] = mpMREA->ReadLong();

    // SCLY
    for (uint32 iLyr = 0; iLyr < mNumLayers; iLyr++)
    {
        uint32 Next = mpMREA->Tell() + LayerSizes[iLyr];
        mpArea->mScriptLayers[iLyr] = CScriptLoader::LoadLayer(*mpMREA, mpArea, mVersion);
        mpMREA->Seek(Next, SEEK_SET);
    }

    // SCGN
    CScriptLayer *pGenLayer = nullptr;

    if (mVersion >= EGame::EchoesDemo)
    {
        mpSectionMgr->ToSection(mScriptGeneratorBlockNum);
        CFourCC SCGN = mpMREA->ReadFourCC();

        if (SCGN != FOURCC('SCGN'))
            errorf("%s [0x%X]: Invalid SCGN magic: %s", *mpMREA->GetSourceString(), mpMREA->Tell() - 4, SCGN.ToString());

        else
        {
            mpMREA->Seek(0x1, SEEK_CUR);
            pGenLayer = CScriptLoader::LoadLayer(*mpMREA, mpArea, mVersion);
        }
    }

    SetUpObjects(pGenLayer);
    delete pGenLayer;
}

void CAreaLoader::ReadLightsPrime()
{
    mpSectionMgr->ToSection(mLightsBlockNum);

    uint32 BabeDead = mpMREA->ReadLong();
    if (BabeDead != 0xbabedead) return;

    mpArea->mLightLayers.resize(2);

    for (uint32 iLyr = 0; iLyr < 2; iLyr++)
    {
        uint32 NumLights = mpMREA->ReadLong();
        mpArea->mLightLayers[iLyr].resize(NumLights);

        for (uint32 iLight = 0; iLight < NumLights; iLight++)
        {
            ELightType Type = ELightType(mpMREA->ReadLong());
            CVector3f Color(*mpMREA);
            CVector3f Position(*mpMREA);
            CVector3f Direction(*mpMREA);
            float Multiplier = mpMREA->ReadFloat();
            float SpotCutoff = mpMREA->ReadFloat();
            mpMREA->Seek(0x9, SEEK_CUR);
            uint32 FalloffType = mpMREA->ReadLong();
            mpMREA->Seek(0x4, SEEK_CUR);

            // Relevant data is read - now we process and form a CLight out of it
            CLight *pLight;

            CColor LightColor = CColor(Color.X, Color.Y, Color.Z, 0.f);
            if (Multiplier < FLT_EPSILON)
                Multiplier = FLT_EPSILON;

            // Local Ambient
            if (Type == ELightType::LocalAmbient)
            {
                Color *= Multiplier;

                // Clamp
                if (Color.X > 1.f) Color.X = 1.f;
                if (Color.Y > 1.f) Color.Y = 1.f;
                if (Color.Z > 1.f) Color.Z = 1.f;
                CColor MultColor(Color.X, Color.Y, Color.Z, 1.f);

                pLight = CLight::BuildLocalAmbient(Position, MultColor);
            }

            // Directional
            else if (Type == ELightType::Directional)
            {
                pLight = CLight::BuildDirectional(Position, Direction, LightColor);
            }

            // Spot
            else if (Type == ELightType::Spot)
            {
                pLight = CLight::BuildSpot(Position, Direction.Normalized(), LightColor, SpotCutoff);

                float DistAttenA = (FalloffType == 0) ? (2.f / Multiplier) : 0.f;
                float DistAttenB = (FalloffType == 1) ? (250.f / Multiplier) : 0.f;
                float DistAttenC = (FalloffType == 2) ? (25000.f / Multiplier) : 0.f;
                pLight->SetDistAtten(DistAttenA, DistAttenB, DistAttenC);
            }

            // Custom
            else
            {
                float DistAttenA = (FalloffType == 0) ? (2.f / Multiplier) : 0.f;
                float DistAttenB = (FalloffType == 1) ? (249.9998f / Multiplier) : 0.f;
                float DistAttenC = (FalloffType == 2) ? (25000.f / Multiplier) : 0.f;

                pLight = CLight::BuildCustom(Position, Direction, LightColor,
                                            DistAttenA, DistAttenB, DistAttenC,
                                            1.f, 0.f, 0.f);
            }

            pLight->SetLayer(iLyr);
            mpArea->mLightLayers[iLyr][iLight] = pLight;
        }
    }
}

// ************ ECHOES ************
void CAreaLoader::ReadHeaderEchoes()
{
    // This function reads the header for Echoes and the Echoes demo disc
    mpArea->mTransform = CTransform4f(*mpMREA);
    mNumMeshes = mpMREA->ReadLong();
    if (mVersion == EGame::Echoes) mNumLayers = mpMREA->ReadLong();
    uint32 numBlocks = mpMREA->ReadLong();

    mGeometryBlockNum = mpMREA->ReadLong();
    mScriptLayerBlockNum = mpMREA->ReadLong();
    mScriptGeneratorBlockNum = mpMREA->ReadLong();
    mCollisionBlockNum = mpMREA->ReadLong();
    mUnknownBlockNum = mpMREA->ReadLong();
    mLightsBlockNum = mpMREA->ReadLong();
    mVisiBlockNum = mpMREA->ReadLong();
    mPathBlockNum = mpMREA->ReadLong();
    mFFFFBlockNum = mpMREA->ReadLong();
    mPTLABlockNum = mpMREA->ReadLong();
    mEGMCBlockNum = mpMREA->ReadLong();
    if (mVersion == EGame::Echoes) mClusters.resize(mpMREA->ReadLong());
    mpMREA->SeekToBoundary(32);

    mpSectionMgr = new CSectionMgrIn(numBlocks, mpMREA);
    mpMREA->SeekToBoundary(32);

    if (mVersion == EGame::Echoes)
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
    mpSectionMgr->ToSection(mScriptLayerBlockNum);
    mpArea->mScriptLayers.resize(mNumLayers);

    // SCLY
    for (uint32 iLyr = 0; iLyr < mNumLayers; iLyr++)
    {
        CFourCC SCLY(*mpMREA);
        if (SCLY != FOURCC('SCLY'))
        {
            errorf("%s [0x%X]: Layer %d - Invalid SCLY magic: %s", *mpMREA->GetSourceString(), mpMREA->Tell() - 4, iLyr, *SCLY.ToString());
            mpSectionMgr->ToNextSection();
            continue;
        }

        mpMREA->Seek(0x5, SEEK_CUR); // Skipping unknown + layer index
        mpArea->mScriptLayers[iLyr] = CScriptLoader::LoadLayer(*mpMREA, mpArea, mVersion);
        mpSectionMgr->ToNextSection();
    }

    // SCGN
    CFourCC SCGN(*mpMREA);
    if (SCGN != FOURCC('SCGN'))
    {
        errorf("%s [0x%X]: Invalid SCGN magic: %s", *mpMREA->GetSourceString(), mpMREA->Tell() - 4, *SCGN.ToString());
        return;
    }

    mpMREA->Seek(0x1, SEEK_CUR); // Skipping unknown
    CScriptLayer *pGeneratedLayer = CScriptLoader::LoadLayer(*mpMREA, mpArea, mVersion);
    SetUpObjects(pGeneratedLayer);
    delete pGeneratedLayer;
}

// ************ CORRUPTION ************
void CAreaLoader::ReadHeaderCorruption()
{
    // This function reads the header for MP3, the MP3 prototype, and DKCR
    mpArea->mTransform = CTransform4f(*mpMREA);
    mNumMeshes = mpMREA->ReadLong();
    mNumLayers = mpMREA->ReadLong();
    uint32 NumSections = mpMREA->ReadLong();
    mClusters.resize(mpMREA->ReadLong());
    uint32 SectionNumberCount = mpMREA->ReadLong();
    mpMREA->SeekToBoundary(32);

    mpSectionMgr = new CSectionMgrIn(NumSections, mpMREA);
    mpMREA->SeekToBoundary(32);

    ReadCompressedBlocks();

    for (uint32 iNum = 0; iNum < SectionNumberCount; iNum++)
    {
        CFourCC Type(*mpMREA);
        uint32 Num = mpMREA->ReadLong();

        if      (Type == "AABB") mBoundingBoxesBlockNum = Num;
        else if (Type == "APTL") mPTLABlockNum = Num;
        else if (Type == "COLI") mCollisionBlockNum = Num;
        else if (Type == "DEPS") mDependenciesBlockNum = Num;
        else if (Type == "EGMC") mEGMCBlockNum = Num;
        else if (Type == "GPUD") mGPUBlockNum = Num;
        else if (Type == "LITE") mLightsBlockNum = Num;
        else if (Type == "PFL2") mPathBlockNum = Num;
        else if (Type == "PVS!") mVisiBlockNum = Num;
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
    mpSectionMgr->ToSection(mGeometryBlockNum);

    // Materials
    mpArea->mpMaterialSet = CMaterialLoader::LoadMaterialSet(*mpMREA, mVersion);
    mpSectionMgr->ToNextSection();

    // Geometry
    std::vector<CModel*> FileModels;
    uint32 CurWOBJSection = 1;
    uint32 CurGPUSection = mGPUBlockNum;

    for (uint32 iMesh = 0; iMesh < mNumMeshes; iMesh++)
    {
        CModel *pWorldModel = CModelLoader::LoadCorruptionWorldModel(*mpMREA, *mpSectionMgr, *mpArea->mpMaterialSet, CurWOBJSection, CurGPUSection, mVersion);
        FileModels.push_back(pWorldModel);

        CurWOBJSection += 4;
        CurGPUSection = mpSectionMgr->CurrentSection();

        // Load surface mesh IDs
        mpSectionMgr->ToSection(CurWOBJSection -  2);
        uint16 NumSurfaces = mpMREA->ReadShort();

        for (uint32 iSurf = 0; iSurf < NumSurfaces; iSurf++)
        {
            mpMREA->Seek(0x2, SEEK_CUR);
            pWorldModel->GetSurface(iSurf)->MeshID = mpMREA->ReadShort();
        }
    }

    std::vector<CModel*> SplitModels;
    CModelLoader::BuildWorldMeshes(FileModels, SplitModels, true);

    for (uint32 iMdl = 0; iMdl < SplitModels.size(); iMdl++)
        mpArea->AddWorldModel(SplitModels[iMdl]);

    mpArea->MergeTerrain();
}

void CAreaLoader::ReadDependenciesCorruption()
{
    mpSectionMgr->ToSection(mDependenciesBlockNum);

    // Read the offsets first so we can read the deps directly into their corresponding arrays
    uint32 NumDeps = mpMREA->ReadLong();
    uint32 DepsStart = mpMREA->Tell();
    mpMREA->Skip(NumDeps * 0xC);

    uint32 NumOffsets = mpMREA->ReadLong();
    std::vector<uint32> Offsets(NumOffsets);

    for (uint32 OffsetIdx = 0; OffsetIdx < NumOffsets; OffsetIdx++)
        Offsets[OffsetIdx] = mpMREA->ReadLong();

    mpMREA->GoTo(DepsStart);

    // Read layer dependencies
    uint32 NumLayers = NumOffsets - 1;
    mpArea->mExtraLayerDeps.resize(NumLayers);

    for (uint32 LayerIdx = 0; LayerIdx < NumLayers; LayerIdx++)
    {
        uint32 NumLayerDeps = Offsets[LayerIdx+1] - Offsets[LayerIdx];
        mpArea->mExtraLayerDeps[LayerIdx].reserve(NumLayerDeps);

        for (uint32 DepIdx = 0; DepIdx < NumLayerDeps; DepIdx++)
        {
            CAssetID AssetID(*mpMREA, EGame::Corruption);
            mpMREA->Skip(4);
            mpArea->mExtraLayerDeps[LayerIdx].push_back(AssetID);
        }
    }

    // Read area dependencies
    uint32 NumAreaDeps = NumDeps - Offsets[NumLayers];
    mpArea->mExtraAreaDeps.reserve(NumAreaDeps);

    for (uint32 DepIdx = 0; DepIdx < NumAreaDeps; DepIdx++)
    {
        CAssetID AssetID(*mpMREA, EGame::Corruption);
        mpMREA->Skip(4);
        mpArea->mExtraAreaDeps.push_back(AssetID);
    }
}

void CAreaLoader::ReadLightsCorruption()
{
    mpSectionMgr->ToSection(mLightsBlockNum);

    uint32 BabeDead = mpMREA->ReadLong();
    if (BabeDead != 0xbabedead) return;

    mpArea->mLightLayers.resize(4);

    for (uint32 iLayer = 0; iLayer < 4; iLayer++)
    {
        uint32 NumLights = mpMREA->ReadLong();
        mpArea->mLightLayers[iLayer].resize(NumLights);

        for (uint32 iLight = 0; iLight < NumLights; iLight++)
        {
            ELightType Type = (ELightType) mpMREA->ReadLong();

            float R = mpMREA->ReadFloat();
            float G = mpMREA->ReadFloat();
            float B = mpMREA->ReadFloat();
            float A = mpMREA->ReadFloat();
            CColor LightColor(R, G, B, A);

            CVector3f Position(*mpMREA);
            CVector3f Direction(*mpMREA);
            mpMREA->Seek(0xC, SEEK_CUR);

            float Multiplier = mpMREA->ReadFloat();
            float SpotCutoff = mpMREA->ReadFloat();
            mpMREA->Seek(0x9, SEEK_CUR);
            uint32 FalloffType = mpMREA->ReadLong();
            mpMREA->Seek(0x18, SEEK_CUR);

            // Relevant data is read - now we process and form a CLight out of it
            CLight *pLight;

            if (Multiplier < FLT_EPSILON)
                Multiplier = FLT_EPSILON;

            // Local Ambient
            if (Type == ELightType::LocalAmbient)
            {
                pLight = CLight::BuildLocalAmbient(Position, LightColor * Multiplier);
            }

            // Directional
            else if (Type == ELightType::Directional)
            {
                pLight = CLight::BuildDirectional(Position, Direction, LightColor);
            }

            // Spot
            else if (Type == ELightType::Spot)
            {
                pLight = CLight::BuildSpot(Position, Direction.Normalized(), LightColor, SpotCutoff);

                float DistAttenA = (FalloffType == 0) ? (2.f / Multiplier) : 0.f;
                float DistAttenB = (FalloffType == 1) ? (250.f / Multiplier) : 0.f;
                float DistAttenC = (FalloffType == 2) ? (25000.f / Multiplier) : 0.f;
                pLight->SetDistAtten(DistAttenA, DistAttenB, DistAttenC);
            }

            // Custom
            else
            {
                float DistAttenA = (FalloffType == 0) ? (2.f / Multiplier) : 0.f;
                float DistAttenB = (FalloffType == 1) ? (249.9998f / Multiplier) : 0.f;
                float DistAttenC = (FalloffType == 2) ? (25000.f / Multiplier) : 0.f;

                pLight = CLight::BuildCustom(Position, Direction, LightColor,
                                            DistAttenA, DistAttenB, DistAttenC,
                                            1.f, 0.f, 0.f);
            }

            pLight->SetLayer(iLayer);
            mpArea->mLightLayers[iLayer][iLight] = pLight;
        }
    }
}

// ************ COMMON ************
void CAreaLoader::ReadCompressedBlocks()
{
    mTotalDecmpSize = 0;

    for (uint32 iClust = 0; iClust < mClusters.size(); iClust++)
    {
        mClusters[iClust].BufferSize = mpMREA->ReadLong();
        mClusters[iClust].DecompressedSize = mpMREA->ReadLong();
        mClusters[iClust].CompressedSize = mpMREA->ReadLong();
        mClusters[iClust].NumSections = mpMREA->ReadLong();
        mTotalDecmpSize += mClusters[iClust].DecompressedSize;

        if (mClusters[iClust].CompressedSize != 0) mpArea->mUsesCompression = true;
    }

    mpMREA->SeekToBoundary(32);
}

void CAreaLoader::Decompress()
{
    // This function decompresses compressed clusters into a buffer.
    // It should be called at the beginning of the first compressed cluster.
    if (mVersion < EGame::Echoes) return;

    // Decompress clusters
    mpDecmpBuffer = new uint8[mTotalDecmpSize];
    uint32 Offset = 0;

    for (uint32 iClust = 0; iClust < mClusters.size(); iClust++)
    {
        SCompressedCluster *pClust = &mClusters[iClust];

        // Is it decompressed already?
        if (mClusters[iClust].CompressedSize == 0)
        {
            mpMREA->ReadBytes(mpDecmpBuffer + Offset, pClust->DecompressedSize);
            Offset += pClust->DecompressedSize;
        }

        else
        {
            uint32 StartOffset = 32 - (mClusters[iClust].CompressedSize % 32); // For some reason they pad the beginning instead of the end
            if (StartOffset != 32)
                mpMREA->Seek(StartOffset, SEEK_CUR);

            std::vector<uint8> CompressedBuf(mClusters[iClust].CompressedSize);
            mpMREA->ReadBytes(CompressedBuf.data(), CompressedBuf.size());

            bool Success = CompressionUtil::DecompressSegmentedData(CompressedBuf.data(), CompressedBuf.size(), mpDecmpBuffer + Offset, pClust->DecompressedSize);
            if (!Success)
                throw "Failed to decompress MREA!";

            Offset += pClust->DecompressedSize;
        }
    }

    TString Source = mpMREA->GetSourceString();
    mpMREA = new CMemoryInStream(mpDecmpBuffer, mTotalDecmpSize, EEndian::BigEndian);
    mpMREA->SetSourceString(Source);
    mpSectionMgr->SetInputStream(mpMREA);
    mHasDecompressedBuffer = true;
}

void CAreaLoader::LoadSectionDataBuffers()
{
   mpArea->mSectionDataBuffers.resize(mpSectionMgr->NumSections());
   mpSectionMgr->ToSection(0);

   for (uint32 iSec = 0; iSec < mpSectionMgr->NumSections(); iSec++)
   {
       uint32 Size = mpSectionMgr->CurrentSectionSize();
       mpArea->mSectionDataBuffers[iSec].resize(Size);
       mpMREA->ReadBytes(mpArea->mSectionDataBuffers[iSec].data(), mpArea->mSectionDataBuffers[iSec].size());
       mpSectionMgr->ToNextSection();
   }
}

void CAreaLoader::ReadCollision()
{
    mpSectionMgr->ToSection(mCollisionBlockNum);
    CCollisionMeshGroup* pAreaCollision = CCollisionLoader::LoadAreaCollision(*mpMREA);
    mpArea->mpCollision = std::unique_ptr<CCollisionMeshGroup>(pAreaCollision);
}

void CAreaLoader::ReadPATH()
{
    mpSectionMgr->ToSection(mPathBlockNum);
    mpArea->mPathID = CAssetID(*mpMREA, mVersion);
}

void CAreaLoader::ReadPTLA()
{
    mpSectionMgr->ToSection(this->mPTLABlockNum);
    mpArea->mPortalAreaID = CAssetID(*mpMREA, mVersion);
}

void CAreaLoader::ReadEGMC()
{
    mpSectionMgr->ToSection(mEGMCBlockNum);
    CAssetID EGMC(*mpMREA, mVersion);
    mpArea->mpPoiToWorldMap = gpResourceStore->LoadResource(EGMC, EResourceType::StaticGeometryMap);
}

void CAreaLoader::SetUpObjects(CScriptLayer *pGenLayer)
{
    // Create instance map
    for (uint32 LayerIdx = 0; LayerIdx < mpArea->NumScriptLayers(); LayerIdx++)
    {
        CScriptLayer *pLayer = mpArea->mScriptLayers[LayerIdx];

        for (uint32 InstIdx = 0; InstIdx < pLayer->NumInstances(); InstIdx++)
        {
            CScriptObject *pInst = pLayer->InstanceByIndex(InstIdx);
            uint32 InstanceID = pInst->InstanceID();
            CScriptObject *pExisting = mpArea->InstanceByID(InstanceID);
            ASSERT(pExisting == nullptr);
            mpArea->mObjectMap[InstanceID] = pInst;
        }
    }

    // Merge objects from the generated layer back into the regular script layers
    if (pGenLayer)
    {
        while (pGenLayer->NumInstances() != 0)
        {
            CScriptObject *pInst = pGenLayer->InstanceByIndex(0);
            uint32 InstanceID = pInst->InstanceID();

            // Check if this is a duplicate of an existing instance (this only happens with DKCR GenericCreature as far as I'm aware)
            if (mpArea->InstanceByID(InstanceID) != nullptr)
            {
                if (pInst->ObjectTypeID() != FOURCC('GCTR'))
                    debugf("Duplicate SCGN object: [%s] %s (%08X)", *pInst->Template()->Name(), *pInst->InstanceName(), pInst->InstanceID());

                pGenLayer->RemoveInstance(pInst);
                delete pInst;
            }

            else
            {
                uint32 LayerIdx = (InstanceID >> 26) & 0x3F;
                pInst->SetLayer( mpArea->ScriptLayer(LayerIdx) );
                mpArea->mObjectMap[InstanceID] = pInst;
            }
        }
    }

    // Iterate over all objects
    for (auto Iter = mpArea->mObjectMap.begin(); Iter != mpArea->mObjectMap.end(); Iter++)
    {
        CScriptObject *pInst = Iter->second;

        // Store outgoing connections
        for (uint32 iCon = 0; iCon < pInst->NumLinks(ELinkType::Outgoing); iCon++)
        {
            CLink *pLink = pInst->Link(ELinkType::Outgoing, iCon);
            mConnectionMap[pLink->ReceiverID()].push_back(pLink);
        }

        // Remove "-component" garbage from MP1 instance names
        if (mVersion <= EGame::Prime)
        {
            TString InstanceName = pInst->InstanceName();

            while (InstanceName.EndsWith("-component"))
                InstanceName = InstanceName.ChopBack(10);

            pInst->SetName(InstanceName);
        }
    }

    // Store connections
    for (auto it = mpArea->mObjectMap.begin(); it != mpArea->mObjectMap.end(); it++)
    {
        uint32 InstanceID = it->first;
        auto iConMap = mConnectionMap.find(InstanceID);

        if (iConMap != mConnectionMap.end())
        {
            CScriptObject *pObj = mpArea->InstanceByID(InstanceID);
            pObj->mInLinks = iConMap->second;
        }
    }
}

// ************ STATIC ************
CGameArea* CAreaLoader::LoadMREA(IInputStream& MREA, CResourceEntry *pEntry)
{
    CAreaLoader Loader;

    // Validation
    if (!MREA.IsValid()) return nullptr;

    uint32 DeadBeef = MREA.ReadLong();
    if (DeadBeef != 0xdeadbeef)
    {
        errorf("%s: Invalid MREA magic: 0x%08X", *MREA.GetSourceString(), DeadBeef);
        return nullptr;
    }

    // Header
    Loader.mpArea = new CGameArea(pEntry);
    uint32 Version = MREA.ReadLong();
    Loader.mVersion = GetFormatVersion(Version);
    Loader.mpMREA = &MREA;

    switch (Loader.mVersion)
    {
        case EGame::PrimeDemo:
        case EGame::Prime:
            Loader.ReadHeaderPrime();
            Loader.ReadGeometryPrime();
            Loader.ReadSCLYPrime();
            Loader.ReadCollision();
            Loader.ReadLightsPrime();
            Loader.ReadPATH();
            break;
        case EGame::EchoesDemo:
            Loader.ReadHeaderEchoes();
            Loader.ReadGeometryPrime();
            Loader.ReadSCLYPrime();
            Loader.ReadCollision();
            Loader.ReadLightsPrime();
            Loader.ReadPATH();
            Loader.ReadPTLA();
            Loader.ReadEGMC();
            break;
        case EGame::Echoes:
            Loader.ReadHeaderEchoes();
            Loader.ReadGeometryPrime();
            Loader.ReadSCLYEchoes();
            Loader.ReadCollision();
            Loader.ReadLightsPrime();
            Loader.ReadPATH();
            Loader.ReadPTLA();
            Loader.ReadEGMC();
            break;
        case EGame::CorruptionProto:
            Loader.ReadHeaderCorruption();
            Loader.ReadGeometryPrime();
            Loader.ReadDependenciesCorruption();
            Loader.ReadSCLYEchoes();
            Loader.ReadCollision();
            Loader.ReadLightsCorruption();
            Loader.ReadPATH();
            Loader.ReadPTLA();
            Loader.ReadEGMC();
            break;
        case EGame::Corruption:
        case EGame::DKCReturns:
            Loader.ReadHeaderCorruption();
            Loader.ReadGeometryCorruption();
            Loader.ReadDependenciesCorruption();
            Loader.ReadSCLYEchoes();
            Loader.ReadCollision();
            if (Loader.mVersion == EGame::Corruption)
            {
                Loader.ReadLightsCorruption();
                Loader.ReadPATH();
                Loader.ReadPTLA();
                Loader.ReadEGMC();
            }
            break;
        default:
            errorf("%s: Unsupported MREA version: 0x%X", *MREA.GetSourceString(), Version);
            Loader.mpArea.Delete();
            return nullptr;
    }

    // Cleanup
    delete Loader.mpSectionMgr;
    return Loader.mpArea;
}

EGame CAreaLoader::GetFormatVersion(uint32 Version)
{
    switch (Version)
    {
        case 0xC: return EGame::PrimeDemo;
        case 0xF: return EGame::Prime;
        case 0x15: return EGame::EchoesDemo;
        case 0x19: return EGame::Echoes;
        case 0x1D: return EGame::CorruptionProto;
        case 0x1E: return EGame::Corruption;
        case 0x20: return EGame::DKCReturns;
        default: return EGame::Invalid;
    }
}
