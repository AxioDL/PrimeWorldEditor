#include "CModelLoader.h"
#include "CMaterialLoader.h"
#include <Common/Log.h>
#include <map>

CModelLoader::CModelLoader()
    : mFlags(EModelLoaderFlag::None)
    , mNumVertices(0)
{
}

CModelLoader::~CModelLoader()
{
}

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
    if (mFlags & EModelLoaderFlag::HalfPrecisionPositions) // 16-bit (DKCR only)
    {
        mPositions.resize(mpSectionMgr->CurrentSectionSize() / 0x6);
        float Divisor = 8192.f; // Might be incorrect! Needs verification via size comparison.

        for (uint32 iVtx = 0; iVtx < mPositions.size(); iVtx++)
        {
            float X = rModel.ReadShort() / Divisor;
            float Y = rModel.ReadShort() / Divisor;
            float Z = rModel.ReadShort() / Divisor;
            mPositions[iVtx] = CVector3f(X, Y, Z);
        }
    }

    else // 32-bit
    {
        mPositions.resize(mpSectionMgr->CurrentSectionSize() / 0xC);

        for (uint32 iVtx = 0; iVtx < mPositions.size(); iVtx++)
            mPositions[iVtx] = CVector3f(rModel);
    }

    mpSectionMgr->ToNextSection();

    // Normals
    if (mFlags & EModelLoaderFlag::HalfPrecisionNormals) // 16-bit
    {
        mNormals.resize(mpSectionMgr->CurrentSectionSize() / 0x6);
        float Divisor = (mVersion < EGame::DKCReturns) ? 32768.f : 16384.f;

        for (uint32 iVtx = 0; iVtx < mNormals.size(); iVtx++)
        {
            float X = rModel.ReadShort() / Divisor;
            float Y = rModel.ReadShort() / Divisor;
            float Z = rModel.ReadShort() / Divisor;
            mNormals[iVtx] = CVector3f(X, Y, Z);
        }
    }
    else // 32-bit
    {
        mNormals.resize(mpSectionMgr->CurrentSectionSize() / 0xC);

        for (uint32 iVtx = 0; iVtx < mNormals.size(); iVtx++)
            mNormals[iVtx] = CVector3f(rModel);
    }

    mpSectionMgr->ToNextSection();

    // Colors
    mColors.resize(mpSectionMgr->CurrentSectionSize() / 4);

    for (uint32 iVtx = 0; iVtx < mColors.size(); iVtx++)
        mColors[iVtx] = CColor(rModel);

    mpSectionMgr->ToNextSection();


    // UVs
    mTex0.resize(mpSectionMgr->CurrentSectionSize() / 0x8);

    for (uint32 iVtx = 0; iVtx < mTex0.size(); iVtx++)
        mTex0[iVtx] = CVector2f(rModel);

    mpSectionMgr->ToNextSection();

    // Lightmap UVs
    if (mFlags & EModelLoaderFlag::LightmapUVs)
    {
        mTex1.resize(mpSectionMgr->CurrentSectionSize() / 0x4);
        float Divisor = (mVersion < EGame::DKCReturns) ? 32768.f : 8192.f;

        for (uint32 iVtx = 0; iVtx < mTex1.size(); iVtx++)
        {
            float X = rModel.ReadShort() / Divisor;
            float Y = rModel.ReadShort() / Divisor;
            mTex1[iVtx] = CVector2f(X, Y);
        }

        mpSectionMgr->ToNextSection();
    }
}

void CModelLoader::LoadSurfaceOffsets(IInputStream& rModel)
{
    mSurfaceCount = rModel.ReadLong();
    mSurfaceOffsets.resize(mSurfaceCount);

    for (uint32 iSurf = 0; iSurf < mSurfaceCount; iSurf++)
        mSurfaceOffsets[iSurf] = rModel.ReadLong();

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

    bool HasAABB = (pSurf->AABox != CAABox::skInfinite);
    CMaterial *pMat = mMaterials[0]->MaterialByIndex(pSurf->MaterialID);

    // Primitive table
    uint8 Flag = rModel.ReadByte();
    uint32 NextSurface = mpSectionMgr->NextOffset();

    while ((Flag != 0) && ((uint32) rModel.Tell() < NextSurface))
    {
        SSurface::SPrimitive Prim;
        Prim.Type = EPrimitiveType(Flag & 0xF8);
        uint16 VertexCount = rModel.ReadShort();

        for (uint16 iVtx = 0; iVtx < VertexCount; iVtx++)
        {
            CVertex Vtx;
            FVertexDescription VtxDesc = pMat->VtxDesc();

            for (uint32 iMtxAttr = 0; iMtxAttr < 8; iMtxAttr++)
                if (VtxDesc & ((uint) EVertexAttribute::PosMtx << iMtxAttr)) rModel.Seek(0x1, SEEK_CUR);

            // Only thing to do here is check whether each attribute is present, and if so, read it.
            // A couple attributes have special considerations; normals can be floats or shorts, as can tex0, depending on vtxfmt.
            // tex0 can also be read from either UV buffer; depends what the material says.

            // Position
            if (VtxDesc & EVertexAttribute::Position)
            {
                uint16 PosIndex = rModel.ReadShort() & 0xFFFF;
                Vtx.Position = mPositions[PosIndex];
                Vtx.ArrayPosition = PosIndex;

                if (!HasAABB) pSurf->AABox.ExpandBounds(Vtx.Position);
            }

            // Normal
            if (VtxDesc & EVertexAttribute::Normal)
                Vtx.Normal = mNormals[rModel.ReadShort() & 0xFFFF];

            // Color
            for (uint32 iClr = 0; iClr < 2; iClr++)
                if (VtxDesc & ((uint) EVertexAttribute::Color0 << iClr))
                    Vtx.Color[iClr] = mColors[rModel.ReadShort() & 0xFFFF];

            // Tex Coords - these are done a bit differently in DKCR than in the Prime series
            if (mVersion < EGame::DKCReturns)
            {
                // Tex0
                if (VtxDesc & EVertexAttribute::Tex0)
                {
                    if ((mFlags & EModelLoaderFlag::LightmapUVs) && (pMat->Options() & EMaterialOption::ShortTexCoord))
                        Vtx.Tex[0] = mTex1[rModel.ReadShort() & 0xFFFF];
                    else
                        Vtx.Tex[0] = mTex0[rModel.ReadShort() & 0xFFFF];
                }

                // Tex1-7
                for (uint32 iTex = 1; iTex < 7; iTex++)
                    if (VtxDesc & ((uint) EVertexAttribute::Tex0 << iTex))
                        Vtx.Tex[iTex] = mTex0[rModel.ReadShort() & 0xFFFF];
            }

            else
            {
                // Tex0-7
                for (uint32 iTex = 0; iTex < 7; iTex++)
                {
                    if (VtxDesc & ((uint) EVertexAttribute::Tex0 << iTex))
                    {
                        if (!mSurfaceUsingTex1)
                            Vtx.Tex[iTex] = mTex0[rModel.ReadShort() & 0xFFFF];
                        else
                            Vtx.Tex[iTex] = mTex1[rModel.ReadShort() & 0xFFFF];
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
    pSurf->MaterialID = rModel.ReadLong();

    rModel.Seek(0xC, SEEK_CUR);
    uint32 ExtraSize = rModel.ReadLong();
    pSurf->ReflectionDirection = CVector3f(rModel);

    if (mVersion >= EGame::EchoesDemo)
        rModel.Seek(0x4, SEEK_CUR); // Skipping unknown values

    bool HasAABox = (ExtraSize >= 0x18); // MREAs have a set of bounding box coordinates here.

    // If this surface has a bounding box, we can just read it here. Otherwise we'll fill it in manually.
    if (HasAABox)
    {
        ExtraSize -= 0x18;
        pSurf->AABox = CAABox(rModel);
    }
    else
        pSurf->AABox = CAABox::skInfinite;

    rModel.Seek(ExtraSize, SEEK_CUR);
    rModel.SeekToBoundary(32);
}

void CModelLoader::LoadSurfaceHeaderDKCR(IInputStream& rModel, SSurface *pSurf)
{
    pSurf->CenterPoint = CVector3f(rModel);
    rModel.Seek(0xE, SEEK_CUR);
    pSurf->MaterialID = (uint32) rModel.ReadShort();
    rModel.Seek(0x2, SEEK_CUR);
    mSurfaceUsingTex1 = (rModel.ReadByte() == 1);
    uint32 ExtraSize = rModel.ReadByte();

    if (ExtraSize > 0)
    {
        ExtraSize -= 0x18;
        pSurf->AABox = CAABox(rModel);
    }
    else
        pSurf->AABox = CAABox::skInfinite;

    rModel.Seek(ExtraSize, SEEK_CUR);
    rModel.SeekToBoundary(32);
}

SSurface* CModelLoader::LoadAssimpMesh(const aiMesh *pkMesh, CMaterialSet *pSet)
{
    // Create vertex description and assign it to material
    CMaterial *pMat = pSet->MaterialByIndex(pkMesh->mMaterialIndex);
    FVertexDescription Desc = pMat->VtxDesc();

    if (Desc == (FVertexDescription) EVertexAttribute::None)
    {
        if (pkMesh->HasPositions()) Desc |= EVertexAttribute::Position;
        if (pkMesh->HasNormals())   Desc |= EVertexAttribute::Normal;

        for (uint32 iUV = 0; iUV < pkMesh->GetNumUVChannels(); iUV++)
            Desc |= ((uint) EVertexAttribute::Tex0 << iUV);

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
        uint32 NumIndices = pkMesh->mFaces[0].mNumIndices;
        if (NumIndices == 1) rPrim.Type = EPrimitiveType::Points;
        else if (NumIndices == 2) rPrim.Type = EPrimitiveType::Lines;
        else if (NumIndices == 3) rPrim.Type = EPrimitiveType::Triangles;

        // Generate bounding box, center point, and reflection projection
        pSurf->CenterPoint = CVector3f::skZero;
        pSurf->ReflectionDirection = CVector3f::skZero;

        for (uint32 iVtx = 0; iVtx < pkMesh->mNumVertices; iVtx++)
        {
            aiVector3D AiPos = pkMesh->mVertices[iVtx];
            pSurf->AABox.ExpandBounds(CVector3f(AiPos.x, AiPos.y, AiPos.z));

            if (pkMesh->HasNormals()) {
                aiVector3D aiNrm = pkMesh->mNormals[iVtx];
                pSurf->ReflectionDirection += CVector3f(aiNrm.x, aiNrm.y, aiNrm.z);
            }
        }
        pSurf->CenterPoint = pSurf->AABox.Center();

        if (pkMesh->HasNormals())
            pSurf->ReflectionDirection /= (float) pkMesh->mNumVertices;
        else
            pSurf->ReflectionDirection = CVector3f(1.f, 0.f, 0.f);

        // Set vertex/triangle count
        pSurf->VertexCount = pkMesh->mNumVertices;
        pSurf->TriangleCount = (rPrim.Type == EPrimitiveType::Triangles ? pkMesh->mNumFaces : 0);

        // Create primitive
        for (uint32 iFace = 0; iFace < pkMesh->mNumFaces; iFace++)
        {
            for (uint32 iIndex = 0; iIndex < NumIndices; iIndex++)
            {
                uint32 Index = pkMesh->mFaces[iFace].mIndices[iIndex];

                // Create vertex and add it to the primitive
                CVertex Vert;
                Vert.ArrayPosition = Index + mNumVertices;

                if (pkMesh->HasPositions())
                {
                    aiVector3D AiPos = pkMesh->mVertices[Index];
                    Vert.Position = CVector3f(AiPos.x, AiPos.y, AiPos.z);
                }

                if (pkMesh->HasNormals())
                {
                    aiVector3D AiNrm = pkMesh->mNormals[Index];
                    Vert.Normal = CVector3f(AiNrm.x, AiNrm.y, AiNrm.z);
                }

                for (uint32 iTex = 0; iTex < pkMesh->GetNumUVChannels(); iTex++)
                {
                    aiVector3D AiTex = pkMesh->mTextureCoords[iTex][Index];
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
CModel* CModelLoader::LoadCMDL(IInputStream& rCMDL, CResourceEntry *pEntry)
{
    CModelLoader Loader;

    // CMDL header - same across the three Primes, but different structure in DKCR
    uint32 Magic = rCMDL.ReadLong();

    uint32 Version, BlockCount, MatSetCount;
    CAABox AABox;

    // 0xDEADBABE - Metroid Prime seres
    if (Magic == 0xDEADBABE)
    {
        Version = rCMDL.ReadLong();
        uint32 Flags = rCMDL.ReadLong();
        AABox = CAABox(rCMDL);
        BlockCount = rCMDL.ReadLong();
        MatSetCount = rCMDL.ReadLong();

        if (Flags & 0x1) Loader.mFlags |= EModelLoaderFlag::Skinned;
        if (Flags & 0x2) Loader.mFlags |= EModelLoaderFlag::HalfPrecisionNormals;
        if (Flags & 0x4) Loader.mFlags |= EModelLoaderFlag::LightmapUVs;
    }

    // 0x9381000A - Donkey Kong Country Returns
    else if (Magic == 0x9381000A)
    {
        Version = Magic & 0xFFFF;
        uint32 Flags = rCMDL.ReadLong();
        AABox = CAABox(rCMDL);
        BlockCount = rCMDL.ReadLong();
        MatSetCount = rCMDL.ReadLong();

        // todo: unknown flags
        Loader.mFlags = EModelLoaderFlag::HalfPrecisionNormals | EModelLoaderFlag::LightmapUVs;
        if (Flags & 0x10) Loader.mFlags |= EModelLoaderFlag::VisibilityGroups;
        if (Flags & 0x20) Loader.mFlags |= EModelLoaderFlag::HalfPrecisionPositions;

        // Visibility group data
        // Skipping for now - should read in eventually
        if (Flags & 0x10)
        {
            rCMDL.Seek(0x4, SEEK_CUR);
            uint32 VisGroupCount = rCMDL.ReadLong();

            for (uint32 iVis = 0; iVis < VisGroupCount; iVis++)
            {
                uint32 NameLength = rCMDL.ReadLong();
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

    CModel *pModel = new CModel(pEntry);
    Loader.mpModel = pModel;
    Loader.mpSectionMgr = new CSectionMgrIn(BlockCount, &rCMDL);
    rCMDL.SeekToBoundary(32);
    Loader.mpSectionMgr->Init();

    // Materials
    Loader.mMaterials.resize(MatSetCount);
    for (uint32 iSet = 0; iSet < MatSetCount; iSet++)
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

    for (uint32 iSurf = 0; iSurf < Loader.mSurfaceCount; iSurf++)
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

CModel* CModelLoader::LoadWorldModel(IInputStream& rMREA, CSectionMgrIn& rBlockMgr, CMaterialSet& rMatSet, EGame Version)
{
    CModelLoader Loader;
    Loader.mpSectionMgr = &rBlockMgr;
    Loader.mVersion = Version;
    Loader.mFlags = EModelLoaderFlag::HalfPrecisionNormals;
    if (Version != EGame::CorruptionProto) Loader.mFlags |= EModelLoaderFlag::LightmapUVs;
    Loader.mMaterials.resize(1);
    Loader.mMaterials[0] = &rMatSet;

    Loader.LoadWorldMeshHeader(rMREA);
    Loader.LoadAttribArrays(rMREA);
    Loader.LoadSurfaceOffsets(rMREA);

    CModel *pModel = new CModel();
    pModel->mMaterialSets.resize(1);
    pModel->mMaterialSets[0] = &rMatSet;
    pModel->mHasOwnMaterials = false;
    pModel->mSurfaces.reserve(Loader.mSurfaceCount);
    pModel->mHasOwnSurfaces = true;

    for (uint32 iSurf = 0; iSurf < Loader.mSurfaceCount; iSurf++)
    {
        SSurface *pSurf = Loader.LoadSurface(rMREA);
        pModel->mSurfaces.push_back(pSurf);
        pModel->mVertexCount += pSurf->VertexCount;
        pModel->mTriangleCount += pSurf->TriangleCount;
    }

    pModel->mAABox = Loader.mAABox;
    return pModel;
}

CModel* CModelLoader::LoadCorruptionWorldModel(IInputStream& rMREA, CSectionMgrIn& rBlockMgr, CMaterialSet& rMatSet, uint32 HeaderSecNum, uint32 GPUSecNum, EGame Version)
{
    CModelLoader Loader;
    Loader.mpSectionMgr = &rBlockMgr;
    Loader.mVersion = Version;
    Loader.mFlags = EModelLoaderFlag::HalfPrecisionNormals;
    Loader.mMaterials.resize(1);
    Loader.mMaterials[0] = &rMatSet;
    if (Version == EGame::DKCReturns) Loader.mFlags |= EModelLoaderFlag::LightmapUVs;

    // Corruption/DKCR MREAs split the mesh header and surface offsets away from the actual geometry data so I need two section numbers to read it
    rBlockMgr.ToSection(HeaderSecNum);
    Loader.LoadWorldMeshHeader(rMREA);
    Loader.LoadSurfaceOffsets(rMREA);
    rBlockMgr.ToSection(GPUSecNum);
    Loader.LoadAttribArrays(rMREA);

    CModel *pModel = new CModel();
    pModel->mMaterialSets.resize(1);
    pModel->mMaterialSets[0] = &rMatSet;
    pModel->mHasOwnMaterials = false;
    pModel->mSurfaces.reserve(Loader.mSurfaceCount);
    pModel->mHasOwnSurfaces = true;

    for (uint32 iSurf = 0; iSurf < Loader.mSurfaceCount; iSurf++)
    {
        SSurface *pSurf = Loader.LoadSurface(rMREA);
        pModel->mSurfaces.push_back(pSurf);
        pModel->mVertexCount += pSurf->VertexCount;
        pModel->mTriangleCount += pSurf->TriangleCount;
    }

    pModel->mAABox = Loader.mAABox;
    return pModel;
}

void CModelLoader::BuildWorldMeshes(const std::vector<CModel*>& rkIn, std::vector<CModel*>& rOut, bool DeleteInputModels)
{
    // This function takes the gigantic models with all surfaces combined from MP2/3/DKCR and splits the surfaces to reform the original uncombined meshes.
    std::map<uint32, CModel*> OutputMap;

    for (uint32 iMdl = 0; iMdl < rkIn.size(); iMdl++)
    {
        CModel *pModel = rkIn[iMdl];
        pModel->mHasOwnSurfaces = false;
        pModel->mHasOwnMaterials = false;

        for (uint32 iSurf = 0; iSurf < pModel->mSurfaces.size(); iSurf++)
        {
            SSurface *pSurf = pModel->mSurfaces[iSurf];
            uint32 ID = (uint32) pSurf->MeshID;
            auto Iter = OutputMap.find(ID);

            // No model for this ID; create one!
            if (Iter == OutputMap.end())
            {
                CModel *pOutMdl = new CModel();
                pOutMdl->mMaterialSets.resize(1);
                pOutMdl->mMaterialSets[0] = pModel->mMaterialSets[0];
                pOutMdl->mHasOwnMaterials = false;
                pOutMdl->mSurfaces.push_back(pSurf);
                pOutMdl->mHasOwnSurfaces = true;
                pOutMdl->mVertexCount = pSurf->VertexCount;
                pOutMdl->mTriangleCount = pSurf->TriangleCount;
                pOutMdl->mAABox.ExpandBounds(pSurf->AABox);

                OutputMap[ID] = pOutMdl;
                rOut.push_back(pOutMdl);
            }

            // Existing model; add this surface to it
            else
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
            delete pModel;
    }
}

CModel* CModelLoader::ImportAssimpNode(const aiNode *pkNode, const aiScene *pkScene, CMaterialSet& rMatSet)
{
    CModelLoader Loader;
    Loader.mpModel = new CModel(&rMatSet, true);
    Loader.mpModel->mSurfaces.reserve(pkNode->mNumMeshes);

    for (uint32 iMesh = 0; iMesh < pkNode->mNumMeshes; iMesh++)
    {
        uint32 MeshIndex = pkNode->mMeshes[iMesh];
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
