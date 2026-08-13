#include "Core/Resource/Factory/CAreaLoader.h"

#include "Core/CompressionUtil.h"
#include "Core/GameProject/CResourceStore.h"
#include "Core/Resource/Area/CGameArea.h"
#include "Core/Resource/Factory/CCollisionLoader.h"
#include "Core/Resource/Factory/CModelLoader.h"
#include "Core/Resource/Factory/CMaterialLoader.h"
#include "Core/Resource/Factory/CScriptLoader.h"
#include "Core/Resource/Factory/CSectionMgrIn.h"
#include "Core/Resource/Model/SSurface.h"
#include "Core/Resource/Script/CLink.h"
#include "Core/Resource/Script/CScriptLayer.h"
#include "Core/Resource/Script/CScriptTemplate.h"

#include <Common/CFourCC.h>
#include <Common/Log.h>
#include <Common/FileIO/CMemoryInStream.h>

#include <algorithm>
#include <cfloat>

CAreaLoader::CAreaLoader() = default;

CAreaLoader::~CAreaLoader()
{
    if (mHasDecompressedBuffer)
        delete mpMREA;
}

// ************ PRIME ************
void CAreaLoader::ReadHeaderPrime()
{
    mpArea->mTransform = CTransform4f(*mpMREA);
    mNumMeshes = mpMREA->ReadU32();
    const auto mNumBlocks = mpMREA->ReadU32();

    mGeometryBlockNum = mpMREA->ReadU32();
    mScriptLayerBlockNum = mpMREA->ReadU32();
    mCollisionBlockNum = mpMREA->ReadU32();
    mUnknownBlockNum = mpMREA->ReadU32();
    mLightsBlockNum = mpMREA->ReadU32();
    mVisiBlockNum = mpMREA->ReadU32();
    mPathBlockNum = mpMREA->ReadU32();
    mOctreeBlockNum = mpMREA->ReadU32();

    mpSectionMgr = std::make_unique<CSectionMgrIn>(mNumBlocks, mpMREA);
    mpMREA->SeekToBoundary(32);
    mpSectionMgr->Init();
    LoadSectionDataBuffers();

    mpArea->mOriginalWorldMeshCount = mNumMeshes;
}

void CAreaLoader::ReadGeometryPrime()
{
    mpSectionMgr->ToSection(mGeometryBlockNum);

    // Materials
    mpArea->mpMaterialSet.reset(CMaterialLoader::LoadMaterialSet(*mpMREA, mVersion, mpArea->Entry()->ResourceStore()));
    mpSectionMgr->ToNextSection();

    // Geometry
    std::vector<std::unique_ptr<CModel>> FileModels;

    for (uint32_t iMesh = 0; iMesh < mNumMeshes; iMesh++)
    {
        if (mVersion <= EGame::Prime)
        {
            mpArea->AddWorldModel(CModelLoader::LoadWorldModel(*mpMREA, *mpSectionMgr, *mpArea->mpMaterialSet, mVersion));
        }
        else // For Echoes+, load surface mesh IDs, then skip to the start of the next mesh
        {
            auto& pModel = FileModels.emplace_back(CModelLoader::LoadWorldModel(*mpMREA, *mpSectionMgr, *mpArea->mpMaterialSet, mVersion));
            const size_t NumSurfaces = mpMREA->ReadU16();

            for (size_t iSurf = 0; iSurf < NumSurfaces; iSurf++)
            {
                mpMREA->Seek(0x2, SEEK_CUR);
                pModel->GetSurface(iSurf)->MeshID = mpMREA->ReadU16();
            }

            mpSectionMgr->ToNextSection();
            mpSectionMgr->ToNextSection();
        }
    }

    // Split meshes
    if (mVersion >= EGame::EchoesDemo)
    {
        std::vector<std::unique_ptr<CModel>> SplitModels;
        CModelLoader::BuildWorldMeshes(FileModels, SplitModels, true);

        for (auto& model : SplitModels)
            mpArea->AddWorldModel(std::move(model));
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
        NLog::Error("{} [0x{:X}]: Invalid SCLY magic: {}", mpMREA->GetSourceString(), mpMREA->Tell() - 4, SCLY.ToString());
        return;
    }
    mpMREA->Seek(mVersion <= EGame::Prime ? 4 : 1, SEEK_CUR); // Skipping unknown value which is always 1

    // Read layer sizes
    mNumLayers = mpMREA->ReadU32();
    mpArea->mScriptLayers.resize(mNumLayers);
    std::vector<uint32_t> LayerSizes(mNumLayers);

    for (auto& layerSize : LayerSizes)
        layerSize = mpMREA->ReadU32();

    // SCLY
    for (size_t iLyr = 0; iLyr < mNumLayers; iLyr++)
    {
        const auto Next = mpMREA->Tell() + LayerSizes[iLyr];
        mpArea->mScriptLayers[iLyr] = CScriptLoader::LoadLayer(*mpMREA, mpArea, mVersion);
        mpMREA->Seek(Next, SEEK_SET);
    }

    // SCGN
    std::unique_ptr<CScriptLayer> pGenLayer;

    if (mVersion >= EGame::EchoesDemo)
    {
        mpSectionMgr->ToSection(mScriptGeneratorBlockNum);
        const CFourCC SCGN(mpMREA->ReadFourCC());

        if (SCGN != FOURCC('SCGN'))
        {
            NLog::Error("{} [0x{:X}]: Invalid SCGN magic: {}", mpMREA->GetSourceString(), mpMREA->Tell() - 4, SCGN.ToString());
        }
        else
        {
            mpMREA->Seek(0x1, SEEK_CUR);
            pGenLayer = CScriptLoader::LoadLayer(*mpMREA, mpArea, mVersion);
        }
    }

    SetUpObjects(pGenLayer.get());
}

void CAreaLoader::ReadLightsPrime()
{
    mpSectionMgr->ToSection(mLightsBlockNum);

    const auto BabeDead = mpMREA->ReadU32();
    if (BabeDead != 0xbabedead)
        return;

    mpArea->mLightLayers.resize(2);

    for (uint32_t iLyr = 0; iLyr < 2; iLyr++)
    {
        const auto NumLights = mpMREA->ReadU32();
        mpArea->mLightLayers[iLyr].resize(NumLights);

        for (uint32_t iLight = 0; iLight < NumLights; iLight++)
        {
            const auto Type = ELightType(mpMREA->ReadU32());
            CVector3f Color(*mpMREA);
            CVector3f Position(*mpMREA);
            CVector3f Direction(*mpMREA);
            float Multiplier = mpMREA->ReadF32();
            const float SpotCutoff = mpMREA->ReadF32();
            mpMREA->Seek(0x9, SEEK_CUR);
            const auto FalloffType = mpMREA->ReadU32();
            mpMREA->Seek(0x4, SEEK_CUR);

            // Relevant data is read - now we process and form a CLight out of it
            CLight pLight;

            const CColor LightColor = CColor(Color.X, Color.Y, Color.Z, 0.f);
            if (Multiplier < FLT_EPSILON)
                Multiplier = FLT_EPSILON;

            // Local Ambient
            if (Type == ELightType::LocalAmbient)
            {
                Color *= Multiplier;

                // Clamp
                Color.X = std::min(Color.X, 1.0f);
                Color.Y = std::min(Color.Y, 1.0f);
                Color.Z = std::min(Color.Z, 1.0f);

                const CColor MultColor(Color.X, Color.Y, Color.Z, 1.f);

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

                const float DistAttenA = (FalloffType == 0) ? (2.f / Multiplier) : 0.f;
                const float DistAttenB = (FalloffType == 1) ? (250.f / Multiplier) : 0.f;
                const float DistAttenC = (FalloffType == 2) ? (25000.f / Multiplier) : 0.f;
                pLight.SetDistAtten(DistAttenA, DistAttenB, DistAttenC);
            }
            else // Custom
            {
                const float DistAttenA = (FalloffType == 0) ? (2.f / Multiplier) : 0.f;
                const float DistAttenB = (FalloffType == 1) ? (250.f / Multiplier) : 0.f;
                const float DistAttenC = (FalloffType == 2) ? (25000.f / Multiplier) : 0.f;

                pLight = CLight::BuildCustom(Position, Direction, LightColor,
                                            DistAttenA, DistAttenB, DistAttenC,
                                            1.f, 0.f, 0.f);
            }

            pLight.SetLayer(iLyr);
            mpArea->mLightLayers[iLyr][iLight] = pLight;
        }
    }
}

// ************ ECHOES ************
void CAreaLoader::ReadHeaderEchoes()
{
    // This function reads the header for Echoes and the Echoes demo disc
    mpArea->mTransform = CTransform4f(*mpMREA);
    mNumMeshes = mpMREA->ReadU32();
    if (mVersion == EGame::Echoes)
        mNumLayers = mpMREA->ReadU32();

    const auto numBlocks = mpMREA->ReadU32();

    mGeometryBlockNum = mpMREA->ReadU32();
    mScriptLayerBlockNum = mpMREA->ReadU32();
    mScriptGeneratorBlockNum = mpMREA->ReadU32();
    mCollisionBlockNum = mpMREA->ReadU32();
    mUnknownBlockNum = mpMREA->ReadU32();
    mLightsBlockNum = mpMREA->ReadU32();
    mVisiBlockNum = mpMREA->ReadU32();
    mPathBlockNum = mpMREA->ReadU32();
    mFFFFBlockNum = mpMREA->ReadU32();
    mPTLABlockNum = mpMREA->ReadU32();
    mEGMCBlockNum = mpMREA->ReadU32();
    if (mVersion == EGame::Echoes)
        mClusters.resize(mpMREA->ReadU32());
    mpMREA->SeekToBoundary(32);

    mpSectionMgr = std::make_unique<CSectionMgrIn>(numBlocks, mpMREA);
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
    for (uint32_t iLyr = 0; iLyr < mNumLayers; iLyr++)
    {
        const CFourCC SCLY(*mpMREA);
        if (SCLY != FOURCC('SCLY'))
        {
            NLog::Error("{} [0x{:X}]: Layer {} - Invalid SCLY magic: {}", mpMREA->GetSourceString(), mpMREA->Tell() - 4, iLyr, SCLY.ToString());
            mpSectionMgr->ToNextSection();
            continue;
        }

        mpMREA->Seek(0x5, SEEK_CUR); // Skipping unknown + layer index
        mpArea->mScriptLayers[iLyr] = CScriptLoader::LoadLayer(*mpMREA, mpArea, mVersion);
        mpSectionMgr->ToNextSection();
    }

    // SCGN
    const CFourCC SCGN(*mpMREA);
    if (SCGN != FOURCC('SCGN'))
    {
        NLog::Error("{} [0x{:X}]: Invalid SCGN magic: {}", mpMREA->GetSourceString(), mpMREA->Tell() - 4, SCGN.ToString());
        return;
    }

    mpMREA->Seek(0x1, SEEK_CUR); // Skipping unknown
    const auto pGeneratedLayer = CScriptLoader::LoadLayer(*mpMREA, mpArea, mVersion);
    SetUpObjects(pGeneratedLayer.get());
}

// ************ CORRUPTION ************
void CAreaLoader::ReadHeaderCorruption()
{
    // This function reads the header for MP3, the MP3 prototype, and DKCR
    mpArea->mTransform = CTransform4f(*mpMREA);
    mNumMeshes = mpMREA->ReadU32();
    mNumLayers = mpMREA->ReadU32();
    const auto NumSections = mpMREA->ReadU32();
    mClusters.resize(mpMREA->ReadU32());
    const auto SectionNumberCount = mpMREA->ReadU32();
    mpMREA->SeekToBoundary(32);

    mpSectionMgr = std::make_unique<CSectionMgrIn>(NumSections, mpMREA);
    mpMREA->SeekToBoundary(32);

    ReadCompressedBlocks();

    for (uint32_t iNum = 0; iNum < SectionNumberCount; iNum++)
    {
        const CFourCC Type(*mpMREA);
        const auto Num = mpMREA->ReadU32();

        if      (Type == CFourCC("AABB")) mBoundingBoxesBlockNum = Num;
        else if (Type == CFourCC("APTL")) mPTLABlockNum = Num;
        else if (Type == CFourCC("COLI")) mCollisionBlockNum = Num;
        else if (Type == CFourCC("DEPS")) mDependenciesBlockNum = Num;
        else if (Type == CFourCC("EGMC")) mEGMCBlockNum = Num;
        else if (Type == CFourCC("GPUD")) mGPUBlockNum = Num;
        else if (Type == CFourCC("LITE")) mLightsBlockNum = Num;
        else if (Type == CFourCC("PFL2")) mPathBlockNum = Num;
        else if (Type == CFourCC("PVS!")) mVisiBlockNum = Num;
        else if (Type == CFourCC("ROCT")) mOctreeBlockNum = Num;
        else if (Type == CFourCC("RSOS")) mRSOBlockNum = Num;
        else if (Type == CFourCC("SOBJ")) mScriptLayerBlockNum = Num;
        else if (Type == CFourCC("SGEN")) mScriptGeneratorBlockNum = Num;
        else if (Type == CFourCC("WOBJ")) mGeometryBlockNum = Num; // note WOBJ can show up multiple times, but is always 0

        mpArea->mSectionNumbers.emplace_back(Type, Num);
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
    mpArea->mpMaterialSet.reset(CMaterialLoader::LoadMaterialSet(*mpMREA, mVersion, mpArea->Entry()->ResourceStore()));
    mpSectionMgr->ToNextSection();

    // Geometry
    std::vector<std::unique_ptr<CModel>> FileModels;
    uint32_t CurWOBJSection = 1;
    uint32_t CurGPUSection = mGPUBlockNum;

    for (uint32_t iMesh = 0; iMesh < mNumMeshes; iMesh++)
    {
        auto& pWorldModel = FileModels.emplace_back(CModelLoader::LoadCorruptionWorldModel(*mpMREA, *mpSectionMgr, *mpArea->mpMaterialSet, CurWOBJSection, CurGPUSection, mVersion));

        CurWOBJSection += 4;
        CurGPUSection = mpSectionMgr->CurrentSection();

        // Load surface mesh IDs
        mpSectionMgr->ToSection(CurWOBJSection -  2);
        const size_t NumSurfaces = mpMREA->ReadU16();

        for (size_t iSurf = 0; iSurf < NumSurfaces; iSurf++)
        {
            mpMREA->Seek(0x2, SEEK_CUR);
            pWorldModel->GetSurface(iSurf)->MeshID = mpMREA->ReadU16();
        }
    }

    std::vector<std::unique_ptr<CModel>> SplitModels;
    CModelLoader::BuildWorldMeshes(FileModels, SplitModels, true);

    for (auto& model : SplitModels)
        mpArea->AddWorldModel(std::move(model));

    mpArea->MergeTerrain();
}

void CAreaLoader::ReadDependenciesCorruption()
{
    mpSectionMgr->ToSection(mDependenciesBlockNum);

    // Read the offsets first so we can read the deps directly into their corresponding arrays
    const auto NumDeps = mpMREA->ReadU32();
    const auto DepsStart = mpMREA->Tell();
    mpMREA->Skip(NumDeps * 0xC);

    const auto NumOffsets = mpMREA->ReadU32();
    std::vector<uint32_t> Offsets(NumOffsets);

    for (auto& offset : Offsets)
        offset = mpMREA->ReadU32();

    mpMREA->GoTo(DepsStart);

    // Read layer dependencies
    const auto NumLayers = NumOffsets - 1;
    mpArea->mExtraLayerDeps.resize(NumLayers);

    for (size_t LayerIdx = 0; LayerIdx < NumLayers; LayerIdx++)
    {
        const auto NumLayerDeps = Offsets[LayerIdx + 1] - Offsets[LayerIdx];
        mpArea->mExtraLayerDeps[LayerIdx].reserve(NumLayerDeps);

        for (uint32_t DepIdx = 0; DepIdx < NumLayerDeps; DepIdx++)
        {
            const CAssetID AssetID(*mpMREA, EGame::Corruption);
            mpMREA->Skip(4);
            mpArea->mExtraLayerDeps[LayerIdx].push_back(AssetID);
        }
    }

    // Read area dependencies
    const auto NumAreaDeps = NumDeps - Offsets[NumLayers];
    mpArea->mExtraAreaDeps.reserve(NumAreaDeps);

    for (uint32_t DepIdx = 0; DepIdx < NumAreaDeps; DepIdx++)
    {
        const CAssetID AssetID(*mpMREA, EGame::Corruption);
        mpMREA->Skip(4);
        mpArea->mExtraAreaDeps.push_back(AssetID);
    }
}

void CAreaLoader::ReadLightsCorruption()
{
    mpSectionMgr->ToSection(mLightsBlockNum);

    const auto BabeDead = mpMREA->ReadU32();
    if (BabeDead != 0xbabedead)
        return;

    mpArea->mLightLayers.resize(4);

    for (uint32_t iLayer = 0; iLayer < 4; iLayer++)
    {
        const auto NumLights = mpMREA->ReadU32();
        mpArea->mLightLayers[iLayer].resize(NumLights);

        for (uint32_t iLight = 0; iLight < NumLights; iLight++)
        {
            const auto Type = static_cast<ELightType>(mpMREA->ReadU32());

            const float R = mpMREA->ReadF32();
            const float G = mpMREA->ReadF32();
            const float B = mpMREA->ReadF32();
            const float A = mpMREA->ReadF32();
            const CColor LightColor(R, G, B, A);

            const CVector3f Position(*mpMREA);
            const CVector3f Direction(*mpMREA);
            mpMREA->Seek(0xC, SEEK_CUR);

            float Multiplier = mpMREA->ReadF32();
            const float SpotCutoff = mpMREA->ReadF32();
            mpMREA->Seek(0x9, SEEK_CUR);
            const auto FalloffType = mpMREA->ReadU32();
            mpMREA->Seek(0x18, SEEK_CUR);

            // Relevant data is read - now we process and form a CLight out of it
            CLight pLight;

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

                const float DistAttenA = (FalloffType == 0) ? (2.f / Multiplier) : 0.f;
                const float DistAttenB = (FalloffType == 1) ? (250.f / Multiplier) : 0.f;
                const float DistAttenC = (FalloffType == 2) ? (25000.f / Multiplier) : 0.f;
                pLight.SetDistAtten(DistAttenA, DistAttenB, DistAttenC);
            }
            else // Custom
            {
                const float DistAttenA = (FalloffType == 0) ? (2.f / Multiplier) : 0.f;
                const float DistAttenB = (FalloffType == 1) ? (250.f / Multiplier) : 0.f;
                const float DistAttenC = (FalloffType == 2) ? (25000.f / Multiplier) : 0.f;

                pLight = CLight::BuildCustom(Position, Direction, LightColor,
                                            DistAttenA, DistAttenB, DistAttenC,
                                            1.f, 0.f, 0.f);
            }

            pLight.SetLayer(iLayer);
            mpArea->mLightLayers[iLayer][iLight] = pLight;
        }
    }
}

// ************ COMMON ************
void CAreaLoader::ReadCompressedBlocks()
{
    mTotalDecmpSize = 0;

    for (auto& cluster : mClusters)
    {
        cluster.BufferSize = mpMREA->ReadU32();
        cluster.DecompressedSize = mpMREA->ReadU32();
        cluster.CompressedSize = mpMREA->ReadU32();
        cluster.NumSections = mpMREA->ReadU32();
        mTotalDecmpSize += cluster.DecompressedSize;

        if (cluster.CompressedSize != 0)
            mpArea->mUsesCompression = true;
    }

    mpMREA->SeekToBoundary(32);
}

void CAreaLoader::Decompress()
{
    // This function decompresses compressed clusters into a buffer.
    // It should be called at the beginning of the first compressed cluster.
    if (mVersion < EGame::Echoes)
        return;

    // Decompress clusters
    mpDecmpBuffer = std::make_unique_for_overwrite<uint8_t[]>(mTotalDecmpSize);
    uint32_t Offset = 0;

    for (auto& cluster : mClusters)
    {
        // Is it decompressed already?
        if (cluster.CompressedSize == 0)
        {
            mpMREA->ReadBytes(mpDecmpBuffer.get() + Offset, cluster.DecompressedSize);
            Offset += cluster.DecompressedSize;
        }
        else
        {
            const uint32_t StartOffset = 32 - (cluster.CompressedSize % 32); // For some reason they pad the beginning instead of the end
            if (StartOffset != 32)
                mpMREA->Seek(StartOffset, SEEK_CUR);

            std::vector<uint8_t> CompressedBuf(cluster.CompressedSize);
            mpMREA->ReadBytes(CompressedBuf.data(), CompressedBuf.size());

            const bool Success = CompressionUtil::DecompressSegmentedData(CompressedBuf.data(), CompressedBuf.size(),
                                                                          mpDecmpBuffer.get() + Offset, cluster.DecompressedSize);
            if (!Success)
                throw "Failed to decompress MREA!";

            Offset += cluster.DecompressedSize;
        }
    }

    TString Source = mpMREA->GetSourceString();
    mpMREA = new CMemoryInStream(mpDecmpBuffer.get(), mTotalDecmpSize, std::endian::big);
    mpMREA->SetSourceString(std::move(Source));
    mpSectionMgr->SetInputStream(mpMREA);
    mHasDecompressedBuffer = true;
}

void CAreaLoader::LoadSectionDataBuffers()
{
   mpArea->mSectionDataBuffers.resize(mpSectionMgr->NumSections());
   mpSectionMgr->ToSection(0);

   for (size_t iSec = 0; iSec < mpSectionMgr->NumSections(); iSec++)
   {
       const uint32_t Size = mpSectionMgr->CurrentSectionSize();
       mpArea->mSectionDataBuffers[iSec].resize(Size);
       mpMREA->ReadBytes(mpArea->mSectionDataBuffers[iSec].data(), mpArea->mSectionDataBuffers[iSec].size());
       mpSectionMgr->ToNextSection();
   }
}

void CAreaLoader::ReadCollision()
{
    mpSectionMgr->ToSection(mCollisionBlockNum);
    mpArea->mpCollision = CCollisionLoader::LoadAreaCollision(*mpMREA);
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
    const CAssetID EGMC(*mpMREA, mVersion);
    mpArea->mpPoiToWorldMap = mpArea->Entry()->ResourceStore()->LoadResource(EGMC, EResourceType::StaticGeometryMap);
}

void CAreaLoader::SetUpObjects(CScriptLayer *pGenLayer)
{
    // Create instance map
    for (const auto& layer : mpArea->mScriptLayers)
    {
        for (auto* instance : layer->Instances())
        {
            const auto InstanceID = instance->InstanceID();
            [[maybe_unused]] CScriptObject* pExisting = mpArea->InstanceByID(InstanceID);
            ASSERT(pExisting == nullptr);
            mpArea->mObjectMap[InstanceID] = instance;
        }
    }

    // Merge objects from the generated layer back into the regular script layers
    if (pGenLayer != nullptr)
    {
        while (pGenLayer->NumInstances() != 0)
        {
            CScriptObject *pInst = pGenLayer->InstanceByIndex(0);
            const auto InstanceID = pInst->InstanceID();

            // Check if this is a duplicate of an existing instance (this only happens with DKCR GenericCreature as far as I'm aware)
            if (mpArea->InstanceByID(InstanceID) != nullptr)
            {
                if (pInst->ObjectTypeID() != FOURCC('GCTR'))
                {
                    NLog::Debug("Duplicate SCGN object: [{}] {} ({:08X})",
                                pInst->Template()->Name(), pInst->InstanceName(), pInst->InstanceID());
                }

                pGenLayer->RemoveInstance(pInst);
                delete pInst;
            }
            else
            {
                const auto LayerIdx = InstanceID.Layer();
                pInst->SetLayer(mpArea->ScriptLayer(LayerIdx));
                mpArea->mObjectMap[InstanceID] = pInst;
            }
        }
    }

    // Iterate over all objects
    for (auto& object : mpArea->mObjectMap)
    {
        CScriptObject *pInst = object.second;

        // Store outgoing connections
        for (size_t iCon = 0; iCon < pInst->NumLinks(ELinkType::Outgoing); iCon++)
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
    for (auto it = mpArea->mObjectMap.begin(); it != mpArea->mObjectMap.end(); ++it)
    {
        const auto InstanceID = it->first;
        const auto iConMap = mConnectionMap.find(InstanceID);

        if (iConMap != mConnectionMap.cend())
        {
            CScriptObject *pObj = mpArea->InstanceByID(InstanceID);
            pObj->mInLinks = iConMap->second;
        }
    }
}

// ************ STATIC ************
std::unique_ptr<CGameArea> CAreaLoader::LoadMREA(IInputStream& MREA, CResourceEntry *pEntry)
{
    CAreaLoader Loader;

    // Validation
    if (!MREA.IsValid())
        return nullptr;

    const auto DeadBeef = MREA.ReadU32();
    if (DeadBeef != 0xdeadbeef)
    {
        NLog::Error("{}: Invalid MREA magic: 0x{:08X}", MREA.GetSourceString(), DeadBeef);
        return nullptr;
    }

    auto ptr = std::make_unique<CGameArea>(pEntry);

    // Header
    Loader.mpArea = ptr.get();
    const auto Version = MREA.ReadU32();
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
            NLog::Error("{}: Unsupported MREA version: 0x{:X}", MREA.GetSourceString(), Version);
            Loader.mpArea.Delete();
            return nullptr;
    }

    return ptr;
}

EGame CAreaLoader::GetFormatVersion(uint32_t Version)
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
