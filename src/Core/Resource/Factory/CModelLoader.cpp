#include "CModelLoader.h"
#include "CMaterialLoader.h"
#include "Core/Log.h"

CModelLoader::CModelLoader()
{
    mFlags = eNoFlags;
    mNumVertices = 0;
}

CModelLoader::~CModelLoader()
{
}

void CModelLoader::LoadWorldMeshHeader(IInputStream &Model)
{
    // I don't really have any need for most of this data, so
    Model.Seek(0x34, SEEK_CUR);
    mAABox = CAABox(Model);
    mpBlockMgr->ToNextBlock();
}

void CModelLoader::LoadAttribArrays(IInputStream& Model)
{
    // Positions
    if (mFlags & eShortPositions) // Shorts (DKCR only)
    {
        mPositions.resize(mpBlockMgr->CurrentBlockSize() / 0x6);
        float Divisor = 8192.f; // Might be incorrect! Needs verification via size comparison.

        for (u32 iVtx = 0; iVtx < mPositions.size(); iVtx++)
        {
            float x = Model.ReadShort() / Divisor;
            float y = Model.ReadShort() / Divisor;
            float z = Model.ReadShort() / Divisor;
            mPositions[iVtx] = CVector3f(x, y, z);
        }
    }

    else // Floats
    {
        mPositions.resize(mpBlockMgr->CurrentBlockSize() / 0xC);

        for (u32 iVtx = 0; iVtx < mPositions.size(); iVtx++)
            mPositions[iVtx] = CVector3f(Model);
    }

    mpBlockMgr->ToNextBlock();

    // Normals
    if (mFlags & eShortNormals) // Shorts
    {
        mNormals.resize(mpBlockMgr->CurrentBlockSize() / 0x6);
        float Divisor = (mVersion < eReturns) ? 32768.f : 16384.f;

        for (u32 iVtx = 0; iVtx < mNormals.size(); iVtx++)
        {
            float x = Model.ReadShort() / Divisor;
            float y = Model.ReadShort() / Divisor;
            float z = Model.ReadShort() / Divisor;
            mNormals[iVtx] = CVector3f(x, y, z);
        }
    }
    else // Floats
    {
        mNormals.resize(mpBlockMgr->CurrentBlockSize() / 0xC);

        for (u32 iVtx = 0; iVtx < mNormals.size(); iVtx++)
            mNormals[iVtx] = CVector3f(Model);
    }

    mpBlockMgr->ToNextBlock();

    // Colors
    mColors.resize(mpBlockMgr->CurrentBlockSize() / 4);

    for (u32 iVtx = 0; iVtx < mColors.size(); iVtx++)
        mColors[iVtx] = CColor(Model);

    mpBlockMgr->ToNextBlock();


    // Float UVs
    mTex0.resize(mpBlockMgr->CurrentBlockSize() / 0x8);

    for (u32 iVtx = 0; iVtx < mTex0.size(); iVtx++)
        mTex0[iVtx] = CVector2f(Model);

    mpBlockMgr->ToNextBlock();

    // Short UVs
    if (mFlags & eHasTex1)
    {
        mTex1.resize(mpBlockMgr->CurrentBlockSize() / 0x4);
        float Divisor = (mVersion < eReturns) ? 32768.f : 8192.f;

        for (u32 iVtx = 0; iVtx < mTex1.size(); iVtx++)
        {
            float x = Model.ReadShort() / Divisor;
            float y = Model.ReadShort() / Divisor;
            mTex1[iVtx] = CVector2f(x, y);
        }

        mpBlockMgr->ToNextBlock();
    }
}

void CModelLoader::LoadSurfaceOffsets(IInputStream& Model)
{
    mSurfaceCount = Model.ReadLong();
    mSurfaceOffsets.resize(mSurfaceCount);

    for (u32 iSurf = 0; iSurf < mSurfaceCount; iSurf++)
        mSurfaceOffsets[iSurf] = Model.ReadLong();

    mpBlockMgr->ToNextBlock();
}

SSurface* CModelLoader::LoadSurface(IInputStream& Model)
{
    SSurface *pSurf = new SSurface;

    // Surface header
    if (mVersion  < eReturns)
        LoadSurfaceHeaderPrime(Model, pSurf);
    else
        LoadSurfaceHeaderDKCR(Model, pSurf);

    bool HasAABB = (pSurf->AABox != CAABox::skInfinite);
    CMaterial *pMat = mMaterials[0]->MaterialByIndex(pSurf->MaterialID);

    // Primitive table
    u8 Flag = Model.ReadByte();
    u32 NextSurface = mpBlockMgr->NextOffset();

    while ((Flag != 0) && ((u32) Model.Tell() < NextSurface))
    {
        SSurface::SPrimitive Prim;
        Prim.Type = EGXPrimitiveType(Flag & 0xF8);
        u16 VertexCount = Model.ReadShort();

        for (u16 iVtx = 0; iVtx < VertexCount; iVtx++)
        {
            CVertex Vtx;
            FVertexDescription VtxDesc = pMat->VtxDesc();

            for (u32 iMtxAttr = 0; iMtxAttr < 8; iMtxAttr++)
                if (VtxDesc & (ePosMtx << iMtxAttr)) Model.Seek(0x1, SEEK_CUR);

            // Only thing to do here is check whether each attribute is present, and if so, read it.
            // A couple attributes have special considerations; normals can be floats or shorts, as can tex0, depending on vtxfmt.
            // tex0 can also be read from either UV buffer; depends what the material says.

            // Position
            if (VtxDesc & ePosition)
            {
                u16 PosIndex = Model.ReadShort() & 0xFFFF;
                Vtx.Position = mPositions[PosIndex];
                Vtx.ArrayPosition = PosIndex;

                if (!HasAABB) pSurf->AABox.ExpandBounds(Vtx.Position);
            }

            // Normal
            if (VtxDesc & eNormal)
                Vtx.Normal = mNormals[Model.ReadShort() & 0xFFFF];

            // Color
            for (u32 c = 0; c < 2; c++)
                if (VtxDesc & (eColor0 << (c * 2)))
                    Vtx.Color[c] = mColors[Model.ReadShort() & 0xFFFF];

            // Tex Coords - these are done a bit differently in DKCR than in the Prime series
            if (mVersion < eReturns)
            {
                // Tex0
                if (VtxDesc & eTex0)
                {
                    if ((mFlags & eHasTex1) && (pMat->Options() & CMaterial::eShortTexCoord))
                        Vtx.Tex[0] = mTex1[Model.ReadShort() & 0xFFFF];
                    else
                        Vtx.Tex[0] = mTex0[Model.ReadShort() & 0xFFFF];
                }

                // Tex1-7
                for (u32 iTex = 1; iTex < 7; iTex++)
                    if (VtxDesc & (eTex0 << (iTex * 2)))
                        Vtx.Tex[iTex] = mTex0[Model.ReadShort() & 0xFFFF];
            }

            else
            {
                // Tex0-7
                for (u32 iTex = 0; iTex < 7; iTex++)
                {
                    if (VtxDesc & (eTex0 << iTex * 2))
                    {
                        if (!mSurfaceUsingTex1)
                            Vtx.Tex[iTex] = mTex0[Model.ReadShort() & 0xFFFF];
                        else
                            Vtx.Tex[iTex] = mTex1[Model.ReadShort() & 0xFFFF];
                    }
                }
            }

            Prim.Vertices.push_back(Vtx);
        } // Vertex array end

        // Update vertex/triangle count
        pSurf->VertexCount += VertexCount;

        switch (Prim.Type)
        {
            case eGX_Triangles:
                pSurf->TriangleCount += VertexCount / 3;
                break;
            case eGX_TriangleFan:
            case eGX_TriangleStrip:
                pSurf->TriangleCount += VertexCount - 2;
                break;
        }

        pSurf->Primitives.push_back(Prim);
        Flag = Model.ReadByte();
    } // Primitive table end

    mpBlockMgr->ToNextBlock();
    return pSurf;
}

void CModelLoader::LoadSurfaceHeaderPrime(IInputStream& Model, SSurface *pSurf)
{
    pSurf->CenterPoint = CVector3f(Model);
    pSurf->MaterialID = Model.ReadLong();

    Model.Seek(0xC, SEEK_CUR);
    u32 ExtraSize = Model.ReadLong();
    pSurf->ReflectionDirection = CVector3f(Model);
    if (mVersion >= eEchoesDemo) Model.Seek(0x4, SEEK_CUR); // Extra values in Echoes. Not sure what they are.
    bool HasAABox = (ExtraSize >= 0x18); // MREAs have a set of bounding box coordinates here.

    // If this surface has a bounding box, we can just read it here. Otherwise we'll fill it in manually.
    if (HasAABox)
    {
        ExtraSize -= 0x18;
        pSurf->AABox = CAABox(Model);
    }
    else
        pSurf->AABox = CAABox::skInfinite;

    Model.Seek(ExtraSize, SEEK_CUR);
    Model.SeekToBoundary(32);
}

void CModelLoader::LoadSurfaceHeaderDKCR(IInputStream& Model, SSurface *pSurf)
{
    pSurf->CenterPoint = CVector3f(Model);
    Model.Seek(0xE, SEEK_CUR);
    pSurf->MaterialID = (u32) Model.ReadShort();
    Model.Seek(0x2, SEEK_CUR);
    mSurfaceUsingTex1 = (Model.ReadByte() == 1);
    u32 ExtraSize = Model.ReadByte();

    if (ExtraSize > 0)
    {
        ExtraSize -= 0x18;
        pSurf->AABox = CAABox(Model);
    }
    else
        pSurf->AABox = CAABox::skInfinite;

    Model.Seek(ExtraSize, SEEK_CUR);
    Model.SeekToBoundary(32);
}

SSurface* CModelLoader::LoadAssimpMesh(const aiMesh *pMesh, CMaterialSet *pSet)
{
    // Create vertex description and assign it to material
    CMaterial *pMat = pSet->MaterialByIndex(pMesh->mMaterialIndex);
    FVertexDescription desc = pMat->VtxDesc();

    if (desc == eNoAttributes)
    {
        if (pMesh->HasPositions()) desc |= ePosition;
        if (pMesh->HasNormals())   desc |= eNormal;

        for (u32 iUV = 0; iUV < pMesh->GetNumUVChannels(); iUV++)
            desc |= (eTex0 << (iUV * 2));

        pMat->SetVertexDescription(desc);

        // TEMP - disable dynamic lighting on geometry with no normals
        if (!pMesh->HasNormals())
        {
            pMat->SetLightingEnabled(false);
            pMat->Pass(0)->SetColorInputs(eZeroRGB, eOneRGB, eKonstRGB, eZeroRGB);
            pMat->Pass(0)->SetRasSel(eRasColorNull);
        }
    }

    // Create surface
    SSurface *pSurf = new SSurface();
    pSurf->MaterialID = pMesh->mMaterialIndex;

    if (pMesh->mNumFaces > 0)
    {
        pSurf->Primitives.resize(1);
        SSurface::SPrimitive& prim = pSurf->Primitives[0];

        // Check primitive type on first face
        u32 numIndices = pMesh->mFaces[0].mNumIndices;
        if (numIndices == 1) prim.Type = eGX_Points;
        else if (numIndices == 2) prim.Type = eGX_Lines;
        else if (numIndices == 3) prim.Type = eGX_Triangles;

        // Generate bounding box, center point, and reflection projection
        pSurf->CenterPoint = CVector3f::skZero;
        pSurf->ReflectionDirection = CVector3f::skZero;

        for (u32 iVtx = 0; iVtx < pMesh->mNumVertices; iVtx++)
        {
            aiVector3D aiPos = pMesh->mVertices[iVtx];
            pSurf->AABox.ExpandBounds(CVector3f(aiPos.x, aiPos.y, aiPos.z));

            if (pMesh->HasNormals()) {
                aiVector3D aiNrm = pMesh->mNormals[iVtx];
                pSurf->ReflectionDirection += CVector3f(aiNrm.x, aiNrm.y, aiNrm.z);
            }
        }
        pSurf->CenterPoint = pSurf->AABox.Center();

        if (pMesh->HasNormals())
            pSurf->ReflectionDirection /= (float) pMesh->mNumVertices;
        else
            pSurf->ReflectionDirection = CVector3f(1.f, 0.f, 0.f);

        // Set vertex/triangle count
        pSurf->VertexCount = pMesh->mNumVertices;
        pSurf->TriangleCount = (prim.Type == eGX_Triangles ? pMesh->mNumFaces : 0);

        // Create primitive
        for (u32 iFace = 0; iFace < pMesh->mNumFaces; iFace++)
        {
            for (u32 iIndex = 0; iIndex < numIndices; iIndex++)
            {
                u32 index = pMesh->mFaces[iFace].mIndices[iIndex];

                // Create vertex and add it to the primitive
                CVertex vert;
                vert.ArrayPosition = index + mNumVertices;

                if (pMesh->HasPositions()) {
                    aiVector3D aiPos = pMesh->mVertices[index];
                    vert.Position = CVector3f(aiPos.x, aiPos.y, aiPos.z);
                }

                if (pMesh->HasNormals()) {
                    aiVector3D aiNrm = pMesh->mNormals[index];
                    vert.Normal = CVector3f(aiNrm.x, aiNrm.y, aiNrm.z);
                }

                for (u32 iTex = 0; iTex < pMesh->GetNumUVChannels(); iTex++) {
                    aiVector3D aiTex = pMesh->mTextureCoords[iTex][index];
                    vert.Tex[iTex] = CVector2f(aiTex.x, aiTex.y);
                }

                prim.Vertices.push_back(vert);
            }
        }

        mNumVertices += pMesh->mNumVertices;
    }

    return pSurf;
}

// ************ STATIC ************
CModel* CModelLoader::LoadCMDL(IInputStream& CMDL)
{
    CModelLoader Loader;
    Log::Write("Loading " + CMDL.GetSourceString());

    // CMDL header - same across the three Primes, but different structure in DKCR
    u32 Magic = CMDL.ReadLong();

    u32 Version, BlockCount, MatSetCount;
    CAABox AABox;

    // 0xDEADBABE - Metroid Prime seres
    if (Magic == 0xDEADBABE)
    {
        Version = CMDL.ReadLong();
        u32 Flags = CMDL.ReadLong();
        AABox = CAABox(CMDL);
        BlockCount = CMDL.ReadLong();
        MatSetCount = CMDL.ReadLong();

        if (Flags & 0x2) Loader.mFlags |= eShortNormals;
        if (Flags & 0x4) Loader.mFlags |= eHasTex1;
    }

    // 0x9381000A - Donkey Kong Country Returns
    else if (Magic == 0x9381000A)
    {
        Version = Magic & 0xFFFF;
        u32 Flags = CMDL.ReadLong();
        AABox = CAABox(CMDL);
        BlockCount = CMDL.ReadLong();
        MatSetCount = CMDL.ReadLong();

        // todo: unknown flags
        Loader.mFlags = eShortNormals | eHasTex1;
        if (Flags & 0x10) Loader.mFlags |= eHasVisGroups;
        if (Flags & 0x20) Loader.mFlags |= eShortPositions;

        // Visibility group data
        // Skipping for now - should read in eventually
        if (Flags & 0x10)
        {
            CMDL.Seek(0x4, SEEK_CUR);
            u32 VisGroupCount = CMDL.ReadLong();

            for (u32 iVis = 0; iVis < VisGroupCount; iVis++)
            {
                u32 NameLength = CMDL.ReadLong();
                CMDL.Seek(NameLength, SEEK_CUR);
            }

            CMDL.Seek(0x14, SEEK_CUR); // no clue what any of this is!
        }
    }

    else
    {
        Log::FileError(CMDL.GetSourceString(), "Invalid CMDL magic: " + TString::HexString(Magic));
        return nullptr;
    }

    // The rest is common to all CMDL versions
    Loader.mVersion = GetFormatVersion(Version);

    if (Loader.mVersion == eUnknownVersion)
    {
        Log::FileError(CMDL.GetSourceString(), "Unsupported CMDL version: " + TString::HexString(Magic));
        return nullptr;
    }

    CModel *pModel = new CModel();
    Loader.mpModel = pModel;
    Loader.mpBlockMgr = new CBlockMgrIn(BlockCount, &CMDL);
    CMDL.SeekToBoundary(32);
    Loader.mpBlockMgr->Init();

    // Materials
    Loader.mMaterials.resize(MatSetCount);
    for (u32 iMat = 0; iMat < MatSetCount; iMat++)
    {
        Loader.mMaterials[iMat] = CMaterialLoader::LoadMaterialSet(CMDL, Loader.mVersion);

        if (Loader.mVersion < eCorruptionProto)
            Loader.mpBlockMgr->ToNextBlock();
    }

    pModel->mMaterialSets = Loader.mMaterials;
    pModel->mHasOwnMaterials = true;
    if (Loader.mVersion >= eCorruptionProto) Loader.mpBlockMgr->ToNextBlock();

    // Mesh
    Loader.LoadAttribArrays(CMDL);
    Loader.LoadSurfaceOffsets(CMDL);
    pModel->mSurfaces.reserve(Loader.mSurfaceCount);

    for (u32 iSurf = 0; iSurf < Loader.mSurfaceCount; iSurf++)
    {
        SSurface *pSurf = Loader.LoadSurface(CMDL);
        pModel->mSurfaces.push_back(pSurf);
        pModel->mVertexCount += pSurf->VertexCount;
        pModel->mTriangleCount += pSurf->TriangleCount;
    }

    pModel->mAABox = AABox;
    pModel->mHasOwnSurfaces = true;

    // Cleanup
    delete Loader.mpBlockMgr;
    return pModel;
}

CModel* CModelLoader::LoadWorldModel(IInputStream& MREA, CBlockMgrIn& BlockMgr, CMaterialSet& MatSet, EGame Version)
{
    CModelLoader Loader;
    Loader.mpBlockMgr = &BlockMgr;
    Loader.mVersion = Version;
    Loader.mFlags = eShortNormals;
    if (Version != eCorruptionProto) Loader.mFlags |= eHasTex1;
    Loader.mMaterials.resize(1);
    Loader.mMaterials[0] = &MatSet;

    Loader.LoadWorldMeshHeader(MREA);
    Loader.LoadAttribArrays(MREA);
    Loader.LoadSurfaceOffsets(MREA);

    CModel *pModel = new CModel();
    pModel->mMaterialSets.resize(1);
    pModel->mMaterialSets[0] = &MatSet;
    pModel->mHasOwnMaterials = false;
    pModel->mSurfaces.reserve(Loader.mSurfaceCount);
    pModel->mHasOwnSurfaces = true;

    for (u32 iSurf = 0; iSurf < Loader.mSurfaceCount; iSurf++)
    {
        SSurface *pSurf = Loader.LoadSurface(MREA);
        pModel->mSurfaces.push_back(pSurf);
        pModel->mVertexCount += pSurf->VertexCount;
        pModel->mTriangleCount += pSurf->TriangleCount;
    }

    pModel->mAABox = Loader.mAABox;
    return pModel;
}

CModel* CModelLoader::LoadCorruptionWorldModel(IInputStream &MREA, CBlockMgrIn &BlockMgr, CMaterialSet &MatSet, u32 HeaderSecNum, u32 GPUSecNum, EGame Version)
{
    CModelLoader Loader;
    Loader.mpBlockMgr = &BlockMgr;
    Loader.mVersion = Version;
    Loader.mFlags = eShortNormals;
    Loader.mMaterials.resize(1);
    Loader.mMaterials[0] = &MatSet;
    if (Version == eReturns) Loader.mFlags |= eHasTex1;

    // Corruption/DKCR MREAs split the mesh header and surface offsets away from the actual geometry data so I need two section numbers to read it
    BlockMgr.ToBlock(HeaderSecNum);
    Loader.LoadWorldMeshHeader(MREA);
    Loader.LoadSurfaceOffsets(MREA);
    BlockMgr.ToBlock(GPUSecNum);
    Loader.LoadAttribArrays(MREA);

    CModel *pModel = new CModel();
    pModel->mMaterialSets.resize(1);
    pModel->mMaterialSets[0] = &MatSet;
    pModel->mHasOwnMaterials = false;
    pModel->mSurfaces.reserve(Loader.mSurfaceCount);
    pModel->mHasOwnSurfaces = true;

    for (u32 iSurf = 0; iSurf < Loader.mSurfaceCount; iSurf++)
    {
        SSurface *pSurf = Loader.LoadSurface(MREA);
        pModel->mSurfaces.push_back(pSurf);
        pModel->mVertexCount += pSurf->VertexCount;
        pModel->mTriangleCount += pSurf->TriangleCount;
    }

    pModel->mAABox = Loader.mAABox;
    return pModel;
}

CModel* CModelLoader::ImportAssimpNode(const aiNode *pNode, const aiScene *pScene, CMaterialSet& matSet)
{
    CModelLoader loader;
    loader.mpModel = new CModel(&matSet, true);
    loader.mpModel->mSurfaces.reserve(pNode->mNumMeshes);

    for (u32 iMesh = 0; iMesh < pNode->mNumMeshes; iMesh++)
    {
        u32 meshIndex = pNode->mMeshes[iMesh];
        const aiMesh *pMesh = pScene->mMeshes[meshIndex];
        SSurface *pSurf = loader.LoadAssimpMesh(pMesh, &matSet);

        loader.mpModel->mSurfaces.push_back(pSurf);
        loader.mpModel->mAABox.ExpandBounds(pSurf->AABox);
        loader.mpModel->mVertexCount += pSurf->VertexCount;
        loader.mpModel->mTriangleCount += pSurf->TriangleCount;
    }

    return loader.mpModel;
}

EGame CModelLoader::GetFormatVersion(u32 Version)
{
    switch (Version)
    {
        case 0x2: return ePrime;
        case 0x3: return eEchoesDemo;
        case 0x4: return eEchoes;
        case 0x5: return eCorruption;
        case 0xA: return eReturns;
        default: return eUnknownVersion;
    }
}
