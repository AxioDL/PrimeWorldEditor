#include "CModelLoader.h"
#include "CMaterialLoader.h"
#include <Common/Log.h>
#include <map>

CModelLoader::CModelLoader() = default;

CModelLoader::~CModelLoader() = default;

void CModelLoader::LoadWorldMeshHeader(IInputStream& rModel)
{
    // I don't really have any need for most of this data, so
    rModel.Seek(0x34, SEEK_CUR);
    mAABox = CAABox(rModel);
    mpSectionMgr->ToNextSection();
}

void CModelLoader::LoadAttribArrays(IInputStream& rModel)
{
    // Positions
    if ((mFlags & EModelLoaderFlag::HalfPrecisionPositions) != 0) // 16-bit (DKCR only)
    {
        mPositions.resize(mpSectionMgr->CurrentSectionSize() / 0x6);
        constexpr float Divisor = 8192.f; // Might be incorrect! Needs verification via size comparison.

        for (auto& position : mPositions)
        {
            position.X = static_cast<float>(rModel.ReadShort()) / Divisor;
            position.Y = static_cast<float>(rModel.ReadShort()) / Divisor;
            position.Z = static_cast<float>(rModel.ReadShort()) / Divisor;
        }
    }
    else // 32-bit
    {
        mPositions.resize(mpSectionMgr->CurrentSectionSize() / 0xC);

        for (auto& position : mPositions)
            position = CVector3f(rModel);
    }

    mpSectionMgr->ToNextSection();

    // Normals
    if ((mFlags & EModelLoaderFlag::HalfPrecisionNormals) != 0) // 16-bit
    {
        mNormals.resize(mpSectionMgr->CurrentSectionSize() / 0x6);
        const float Divisor = (mVersion < EGame::DKCReturns) ? 32768.f : 16384.f;

        for (auto& normal : mNormals)
        {
            normal.X = static_cast<float>(rModel.ReadShort()) / Divisor;
            normal.Y = static_cast<float>(rModel.ReadShort()) / Divisor;
            normal.Z = static_cast<float>(rModel.ReadShort()) / Divisor;
        }
    }
    else // 32-bit
    {
        mNormals.resize(mpSectionMgr->CurrentSectionSize() / 0xC);

        for (auto& normal : mNormals)
            normal = CVector3f(rModel);
    }

    mpSectionMgr->ToNextSection();

    // Colors
    mColors.resize(mpSectionMgr->CurrentSectionSize() / 4);
    for (auto& color : mColors)
    {
        color = CColor(rModel);
    }
    mpSectionMgr->ToNextSection();


    // UVs
    mTex0.resize(mpSectionMgr->CurrentSectionSize() / 0x8);
    for (auto& vec : mTex0)
    {
        vec = CVector2f(rModel);
    }
    mpSectionMgr->ToNextSection();

    // Lightmap UVs
    if ((mFlags & EModelLoaderFlag::LightmapUVs) != 0)
    {
        mTex1.resize(mpSectionMgr->CurrentSectionSize() / 0x4);
        const float Divisor = (mVersion < EGame::DKCReturns) ? 32768.f : 8192.f;

        for (auto& vec : mTex1)
        {
            vec.X = static_cast<float>(rModel.ReadShort()) / Divisor;
            vec.Y = static_cast<float>(rModel.ReadShort()) / Divisor;
        }

        mpSectionMgr->ToNextSection();
    }
}

void CModelLoader::LoadSurfaceOffsets(IInputStream& rModel)
{
    mSurfaceCount = rModel.ReadULong();
    mSurfaceOffsets.resize(mSurfaceCount);

    for (size_t iSurf = 0; iSurf < mSurfaceCount; iSurf++)
        mSurfaceOffsets[iSurf] = rModel.ReadULong();

    mpSectionMgr->ToNextSection();
}

SSurface* CModelLoader::LoadSurface(IInputStream& rModel)
{
    SSurface *pSurf = new SSurface;

    // Surface header
    if (mVersion  < EGame::DKCReturns)
        LoadSurfaceHeaderPrime(rModel, pSurf);
    else
        LoadSurfaceHeaderDKCR(rModel, pSurf);

    const bool HasAABB = pSurf->AABox != CAABox::Infinite();
    CMaterial *pMat = mMaterials[0]->MaterialByIndex(pSurf->MaterialID, false);

    // Primitive table
    uint8 Flag = rModel.ReadUByte();
    const uint32 NextSurface = mpSectionMgr->NextOffset();

    while (Flag != 0 && (static_cast<uint32>(rModel.Tell()) < NextSurface))
    {
        SSurface::SPrimitive Prim;
        Prim.Type = EPrimitiveType(Flag & 0xF8);
        const uint16 VertexCount = rModel.ReadUShort();

        for (uint16 iVtx = 0; iVtx < VertexCount; iVtx++)
        {
            CVertex Vtx;
            FVertexDescription VtxDesc = pMat->VtxDesc();

            for (uint32 iMtxAttr = 0; iMtxAttr < 8; iMtxAttr++)
            {
                if ((VtxDesc & static_cast<uint>(EVertexAttribute::PosMtx << iMtxAttr)) != 0)
                    rModel.Seek(0x1, SEEK_CUR);
            }

            // Only thing to do here is check whether each attribute is present, and if so, read it.
            // A couple attributes have special considerations; normals can be floats or shorts, as can tex0, depending on vtxfmt.
            // tex0 can also be read from either UV buffer; depends what the material says.

            // Position
            if ((VtxDesc & EVertexAttribute::Position) != 0)
            {
                const uint16 PosIndex = rModel.ReadUShort() & 0xFFFF;
                Vtx.Position = mPositions[PosIndex];
                Vtx.ArrayPosition = PosIndex;

                if (!HasAABB)
                    pSurf->AABox.ExpandBounds(Vtx.Position);
            }

            // Normal
            if ((VtxDesc & EVertexAttribute::Normal) != 0)
                Vtx.Normal = mNormals[rModel.ReadUShort() & 0xFFFF];

            // Color
            for (size_t iClr = 0; iClr < Vtx.Color.size(); iClr++)
            {
                if ((VtxDesc & static_cast<uint32>(EVertexAttribute::Color0 << iClr)) != 0)
                    Vtx.Color[iClr] = mColors[rModel.ReadUShort() & 0xFFFF];
            }

            // Tex Coords - these are done a bit differently in DKCR than in the Prime series
            if (mVersion < EGame::DKCReturns)
            {
                // Tex0
                if ((VtxDesc & EVertexAttribute::Tex0) != 0)
                {
                    if ((mFlags & EModelLoaderFlag::LightmapUVs) != 0 && (pMat->Options() & EMaterialOption::ShortTexCoord) != 0)
                        Vtx.Tex[0] = mTex1[rModel.ReadUShort() & 0xFFFF];
                    else
                        Vtx.Tex[0] = mTex0[rModel.ReadUShort() & 0xFFFF];
                }

                // Tex1-7
                for (size_t iTex = 1; iTex < 7; iTex++)
                {
                    if ((VtxDesc & static_cast<uint32>(EVertexAttribute::Tex0 << iTex)) != 0)
                        Vtx.Tex[iTex] = mTex0[rModel.ReadUShort() & 0xFFFF];
                }
            }
            else
            {
                // Tex0-7
                for (size_t iTex = 0; iTex < 7; iTex++)
                {
                    if ((VtxDesc & static_cast<uint32>(EVertexAttribute::Tex0 << iTex)) != 0)
                    {
                        if (!mSurfaceUsingTex1)
                            Vtx.Tex[iTex] = mTex0[rModel.ReadUShort() & 0xFFFF];
                        else
                            Vtx.Tex[iTex] = mTex1[rModel.ReadUShort() & 0xFFFF];
                    }
                }
            }

            Prim.Vertices.push_back(Vtx);
        } // Vertex array end

        // Update vertex/triangle count
        pSurf->VertexCount += VertexCount;

        switch (Prim.Type)
        {
            case EPrimitiveType::Triangles:
                pSurf->TriangleCount += VertexCount / 3;
                break;
            case EPrimitiveType::TriangleFan:
            case EPrimitiveType::TriangleStrip:
                pSurf->TriangleCount += VertexCount - 2;
                break;
            default:
                break;
        }

        pSurf->Primitives.push_back(Prim);
        Flag = rModel.ReadByte();
    } // Primitive table end

    mpSectionMgr->ToNextSection();
    return pSurf;
}

void CModelLoader::LoadSurfaceHeaderPrime(IInputStream& rModel, SSurface *pSurf)
{
    pSurf->CenterPoint = CVector3f(rModel);
    pSurf->MaterialID = rModel.ReadULong();

    rModel.Seek(0xC, SEEK_CUR);
    uint32 ExtraSize = rModel.ReadULong();
    pSurf->ReflectionDirection = CVector3f(rModel);

    if (mVersion >= EGame::EchoesDemo)
        rModel.Seek(0x4, SEEK_CUR); // Skipping unknown values

    const bool HasAABox = (ExtraSize >= 0x18); // MREAs have a set of bounding box coordinates here.

    // If this surface has a bounding box, we can just read it here. Otherwise we'll fill it in manually.
    if (HasAABox)
    {
        ExtraSize -= 0x18;
        pSurf->AABox = CAABox(rModel);
    }
    else
    {
        pSurf->AABox = CAABox::Infinite();
    }

    rModel.Seek(ExtraSize, SEEK_CUR);
    rModel.SeekToBoundary(32);
}

void CModelLoader::LoadSurfaceHeaderDKCR(IInputStream& rModel, SSurface *pSurf)
{
    pSurf->CenterPoint = CVector3f(rModel);
    rModel.Seek(0xE, SEEK_CUR);
    pSurf->MaterialID = rModel.ReadUShort();
    rModel.Seek(0x2, SEEK_CUR);
    mSurfaceUsingTex1 = rModel.ReadUByte() == 1;
    uint32 ExtraSize = rModel.ReadUByte();

    if (ExtraSize > 0)
    {
        ExtraSize -= 0x18;
        pSurf->AABox = CAABox(rModel);
    }
    else
    {
        pSurf->AABox = CAABox::Infinite();
    }

    rModel.Seek(ExtraSize, SEEK_CUR);
    rModel.SeekToBoundary(32);
}

SSurface* CModelLoader::LoadAssimpMesh(const aiMesh *pkMesh, CMaterialSet *pSet)
{
    // Create vertex description and assign it to material
    CMaterial *pMat = pSet->MaterialByIndex(pkMesh->mMaterialIndex, false);
    FVertexDescription Desc = pMat->VtxDesc();

    if (Desc == static_cast<FVertexDescription>(EVertexAttribute::None))
    {
        if (pkMesh->HasPositions())
            Desc |= EVertexAttribute::Position;
        if (pkMesh->HasNormals())
            Desc |= EVertexAttribute::Normal;

        for (size_t iUV = 0; iUV < pkMesh->GetNumUVChannels(); iUV++)
            Desc |= static_cast<uint32>(EVertexAttribute::Tex0 << iUV);

        pMat->SetVertexDescription(Desc);

        // TEMP - disable dynamic lighting on geometry with no normals
        if (!pkMesh->HasNormals())
        {
            pMat->SetLightingEnabled(false);
            pMat->Pass(0)->SetColorInputs(kZeroRGB, kOneRGB, kKonstRGB, kZeroRGB);
            pMat->Pass(0)->SetRasSel(kRasColorNull);
        }
    }

    // Create surface
    SSurface *pSurf = new SSurface();
    pSurf->MaterialID = pkMesh->mMaterialIndex;

    if (pkMesh->mNumFaces > 0)
    {
        pSurf->Primitives.resize(1);
        SSurface::SPrimitive& rPrim = pSurf->Primitives[0];

        // Check primitive type on first face
        const uint32 NumIndices = pkMesh->mFaces[0].mNumIndices;
        if (NumIndices == 1)
            rPrim.Type = EPrimitiveType::Points;
        else if (NumIndices == 2)
            rPrim.Type = EPrimitiveType::Lines;
        else if (NumIndices == 3)
            rPrim.Type = EPrimitiveType::Triangles;

        // Generate bounding box, center point, and reflection projection
        pSurf->CenterPoint = CVector3f::Zero();
        pSurf->ReflectionDirection = CVector3f::Zero();

        for (size_t iVtx = 0; iVtx < pkMesh->mNumVertices; iVtx++)
        {
            const aiVector3D AiPos = pkMesh->mVertices[iVtx];
            pSurf->AABox.ExpandBounds(CVector3f(AiPos.x, AiPos.y, AiPos.z));

            if (pkMesh->HasNormals()) {
                const aiVector3D aiNrm = pkMesh->mNormals[iVtx];
                pSurf->ReflectionDirection += CVector3f(aiNrm.x, aiNrm.y, aiNrm.z);
            }
        }
        pSurf->CenterPoint = pSurf->AABox.Center();

        if (pkMesh->HasNormals())
            pSurf->ReflectionDirection /= static_cast<float>(pkMesh->mNumVertices);
        else
            pSurf->ReflectionDirection = CVector3f(1.f, 0.f, 0.f);

        // Set vertex/triangle count
        pSurf->VertexCount = pkMesh->mNumVertices;
        pSurf->TriangleCount = (rPrim.Type == EPrimitiveType::Triangles ? pkMesh->mNumFaces : 0);

        // Create primitive
        for (size_t iFace = 0; iFace < pkMesh->mNumFaces; iFace++)
        {
            for (size_t iIndex = 0; iIndex < NumIndices; iIndex++)
            {
                const uint32 Index = pkMesh->mFaces[iFace].mIndices[iIndex];

                // Create vertex and add it to the primitive
                CVertex Vert;
                Vert.ArrayPosition = Index + mNumVertices;

                if (pkMesh->HasPositions())
                {
                    const aiVector3D AiPos = pkMesh->mVertices[Index];
                    Vert.Position = CVector3f(AiPos.x, AiPos.y, AiPos.z);
                }

                if (pkMesh->HasNormals())
                {
                    const aiVector3D AiNrm = pkMesh->mNormals[Index];
                    Vert.Normal = CVector3f(AiNrm.x, AiNrm.y, AiNrm.z);
                }

                for (size_t iTex = 0; iTex < pkMesh->GetNumUVChannels(); iTex++)
                {
                    const aiVector3D AiTex = pkMesh->mTextureCoords[iTex][Index];
                    Vert.Tex[iTex] = CVector2f(AiTex.x, AiTex.y);
                }

                rPrim.Vertices.push_back(Vert);
            }
        }

        mNumVertices += pkMesh->mNumVertices;
    }

    return pSurf;
}

// ************ STATIC ************
std::unique_ptr<CModel> CModelLoader::LoadCMDL(IInputStream& rCMDL, CResourceEntry *pEntry)
{
    CModelLoader Loader;

    // CMDL header - same across the three Primes, but different structure in DKCR
    const uint32 Magic = rCMDL.ReadULong();

    uint32 Version, BlockCount, MatSetCount;
    CAABox AABox;

    // 0xDEADBABE - Metroid Prime seres
    if (Magic == 0xDEADBABE)
    {
        Version = rCMDL.ReadULong();
        const uint32 Flags = rCMDL.ReadULong();
        AABox = CAABox(rCMDL);
        BlockCount = rCMDL.ReadULong();
        MatSetCount = rCMDL.ReadULong();

        if ((Flags & 0x1) != 0)
            Loader.mFlags |= EModelLoaderFlag::Skinned;
        if ((Flags & 0x2) != 0)
            Loader.mFlags |= EModelLoaderFlag::HalfPrecisionNormals;
        if ((Flags & 0x4) != 0)
            Loader.mFlags |= EModelLoaderFlag::LightmapUVs;
    }

    // 0x9381000A - Donkey Kong Country Returns
    else if (Magic == 0x9381000A)
    {
        Version = Magic & 0xFFFF;
        const uint32 Flags = rCMDL.ReadULong();
        AABox = CAABox(rCMDL);
        BlockCount = rCMDL.ReadULong();
        MatSetCount = rCMDL.ReadULong();

        // todo: unknown flags
        Loader.mFlags = EModelLoaderFlag::HalfPrecisionNormals | EModelLoaderFlag::LightmapUVs;
        if ((Flags & 0x10) != 0)
            Loader.mFlags |= EModelLoaderFlag::VisibilityGroups;
        if ((Flags & 0x20) != 0)
            Loader.mFlags |= EModelLoaderFlag::HalfPrecisionPositions;

        // Visibility group data
        // Skipping for now - should read in eventually
        if ((Flags & 0x10) != 0)
        {
            rCMDL.Seek(0x4, SEEK_CUR);
            const uint32 VisGroupCount = rCMDL.ReadULong();

            for (uint32 iVis = 0; iVis < VisGroupCount; iVis++)
            {
                const uint32 NameLength = rCMDL.ReadULong();
                rCMDL.Seek(NameLength, SEEK_CUR);
            }

            rCMDL.Seek(0x14, SEEK_CUR); // no clue what any of this is!
        }
    }
    else
    {
        errorf("%s: Invalid CMDL magic: 0x%08X", *rCMDL.GetSourceString(), Magic);
        return nullptr;
    }

    // The rest is common to all CMDL versions
    Loader.mVersion = GetFormatVersion(Version);

    if (Loader.mVersion == EGame::Invalid)
    {
        errorf("%s: Unsupported CMDL version: 0x%X", *rCMDL.GetSourceString(), Magic);
        return nullptr;
    }

    auto pModel = std::make_unique<CModel>(pEntry);
    Loader.mpModel = pModel.get();
    Loader.mpSectionMgr = new CSectionMgrIn(BlockCount, &rCMDL);
    rCMDL.SeekToBoundary(32);
    Loader.mpSectionMgr->Init();

    // Materials
    Loader.mMaterials.resize(MatSetCount);
    for (size_t iSet = 0; iSet < MatSetCount; iSet++)
    {
        Loader.mMaterials[iSet] = CMaterialLoader::LoadMaterialSet(rCMDL, Loader.mVersion);

        if (Loader.mVersion < EGame::CorruptionProto)
            Loader.mpSectionMgr->ToNextSection();
    }

    pModel->mMaterialSets = Loader.mMaterials;
    pModel->mHasOwnMaterials = true;
    if (Loader.mVersion >= EGame::CorruptionProto) Loader.mpSectionMgr->ToNextSection();

    // Mesh
    Loader.LoadAttribArrays(rCMDL);
    Loader.LoadSurfaceOffsets(rCMDL);
    pModel->mSurfaces.reserve(Loader.mSurfaceCount);

    for (size_t iSurf = 0; iSurf < Loader.mSurfaceCount; iSurf++)
    {
        SSurface *pSurf = Loader.LoadSurface(rCMDL);
        pModel->mSurfaces.push_back(pSurf);
        pModel->mVertexCount += pSurf->VertexCount;
        pModel->mTriangleCount += pSurf->TriangleCount;
    }

    pModel->mAABox = AABox;
    pModel->mHasOwnSurfaces = true;

    // Cleanup
    delete Loader.mpSectionMgr;
    return pModel;
}

std::unique_ptr<CModel> CModelLoader::LoadWorldModel(IInputStream& rMREA, CSectionMgrIn& rBlockMgr, CMaterialSet& rMatSet, EGame Version)
{
    CModelLoader Loader;
    Loader.mpSectionMgr = &rBlockMgr;
    Loader.mVersion = Version;
    Loader.mFlags = EModelLoaderFlag::HalfPrecisionNormals;
    if (Version != EGame::CorruptionProto)
        Loader.mFlags |= EModelLoaderFlag::LightmapUVs;
    Loader.mMaterials.resize(1);
    Loader.mMaterials[0] = &rMatSet;

    Loader.LoadWorldMeshHeader(rMREA);
    Loader.LoadAttribArrays(rMREA);
    Loader.LoadSurfaceOffsets(rMREA);

    auto pModel = std::make_unique<CModel>();
    pModel->mMaterialSets.resize(1);
    pModel->mMaterialSets[0] = &rMatSet;
    pModel->mHasOwnMaterials = false;
    pModel->mSurfaces.reserve(Loader.mSurfaceCount);
    pModel->mHasOwnSurfaces = true;

    for (size_t iSurf = 0; iSurf < Loader.mSurfaceCount; iSurf++)
    {
        SSurface *pSurf = Loader.LoadSurface(rMREA);
        pModel->mSurfaces.push_back(pSurf);
        pModel->mVertexCount += pSurf->VertexCount;
        pModel->mTriangleCount += pSurf->TriangleCount;
    }

    pModel->mAABox = Loader.mAABox;
    return pModel;
}

std::unique_ptr<CModel> CModelLoader::LoadCorruptionWorldModel(IInputStream& rMREA, CSectionMgrIn& rBlockMgr, CMaterialSet& rMatSet, uint32 HeaderSecNum, uint32 GPUSecNum, EGame Version)
{
    CModelLoader Loader;
    Loader.mpSectionMgr = &rBlockMgr;
    Loader.mVersion = Version;
    Loader.mFlags = EModelLoaderFlag::HalfPrecisionNormals;
    Loader.mMaterials.resize(1);
    Loader.mMaterials[0] = &rMatSet;
    if (Version == EGame::DKCReturns)
        Loader.mFlags |= EModelLoaderFlag::LightmapUVs;

    // Corruption/DKCR MREAs split the mesh header and surface offsets away from the actual geometry data so I need two section numbers to read it
    rBlockMgr.ToSection(HeaderSecNum);
    Loader.LoadWorldMeshHeader(rMREA);
    Loader.LoadSurfaceOffsets(rMREA);
    rBlockMgr.ToSection(GPUSecNum);
    Loader.LoadAttribArrays(rMREA);

    auto pModel = std::make_unique<CModel>();
    pModel->mMaterialSets.resize(1);
    pModel->mMaterialSets[0] = &rMatSet;
    pModel->mHasOwnMaterials = false;
    pModel->mSurfaces.reserve(Loader.mSurfaceCount);
    pModel->mHasOwnSurfaces = true;

    for (size_t iSurf = 0; iSurf < Loader.mSurfaceCount; iSurf++)
    {
        SSurface *pSurf = Loader.LoadSurface(rMREA);
        pModel->mSurfaces.push_back(pSurf);
        pModel->mVertexCount += pSurf->VertexCount;
        pModel->mTriangleCount += pSurf->TriangleCount;
    }

    pModel->mAABox = Loader.mAABox;
    return pModel;
}

void CModelLoader::BuildWorldMeshes(std::vector<std::unique_ptr<CModel>>& rkIn, std::vector<std::unique_ptr<CModel>>& rOut, bool DeleteInputModels)
{
    // This function takes the gigantic models with all surfaces combined from MP2/3/DKCR and splits the surfaces to reform the original uncombined meshes.
    std::map<uint32, CModel*> OutputMap;

    for (auto& pModel : rkIn)
    {
        pModel->mHasOwnSurfaces = false;
        pModel->mHasOwnMaterials = false;

        for (SSurface* pSurf : pModel->mSurfaces)
        {
            uint32 ID = static_cast<uint32>(pSurf->MeshID);
            const auto Iter = OutputMap.find(ID);

            // No model for this ID; create one!
            if (Iter == OutputMap.cend())
            {
                auto pOutMdl = std::make_unique<CModel>();
                pOutMdl->mMaterialSets.resize(1);
                pOutMdl->mMaterialSets[0] = pModel->mMaterialSets[0];
                pOutMdl->mHasOwnMaterials = false;
                pOutMdl->mSurfaces.push_back(pSurf);
                pOutMdl->mHasOwnSurfaces = true;
                pOutMdl->mVertexCount = pSurf->VertexCount;
                pOutMdl->mTriangleCount = pSurf->TriangleCount;
                pOutMdl->mAABox.ExpandBounds(pSurf->AABox);

                OutputMap.insert_or_assign(ID, pOutMdl.get());
                rOut.push_back(std::move(pOutMdl));
            }
            else // Existing model; add this surface to it
            {
                CModel *pOutMdl = Iter->second;
                pOutMdl->mSurfaces.push_back(pSurf);
                pOutMdl->mVertexCount += pSurf->VertexCount;
                pOutMdl->mTriangleCount += pSurf->TriangleCount;
                pOutMdl->mAABox.ExpandBounds(pSurf->AABox);
            }
        }

        // Done with this model, should we delete it?
        if (DeleteInputModels)
            pModel.reset();
    }
}

CModel* CModelLoader::ImportAssimpNode(const aiNode *pkNode, const aiScene *pkScene, CMaterialSet& rMatSet)
{
    CModelLoader Loader;
    Loader.mpModel = new CModel(&rMatSet, true);
    Loader.mpModel->mSurfaces.reserve(pkNode->mNumMeshes);

    for (size_t iMesh = 0; iMesh < pkNode->mNumMeshes; iMesh++)
    {
        const uint32 MeshIndex = pkNode->mMeshes[iMesh];
        const aiMesh *pkMesh = pkScene->mMeshes[MeshIndex];
        SSurface *pSurf = Loader.LoadAssimpMesh(pkMesh, &rMatSet);

        Loader.mpModel->mSurfaces.push_back(pSurf);
        Loader.mpModel->mAABox.ExpandBounds(pSurf->AABox);
        Loader.mpModel->mVertexCount += pSurf->VertexCount;
        Loader.mpModel->mTriangleCount += pSurf->TriangleCount;
    }

    return Loader.mpModel;
}

EGame CModelLoader::GetFormatVersion(uint32 Version)
{
    switch (Version)
    {
        case 0x2: return EGame::Prime;
        case 0x3: return EGame::EchoesDemo;
        case 0x4: return EGame::Echoes;
        case 0x5: return EGame::Corruption;
        case 0xA: return EGame::DKCReturns;
        default: return EGame::Invalid;
    }
}
