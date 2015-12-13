#include "CCollisionLoader.h"
#include <Core/Log.h>
#include <iostream>

CCollisionLoader::CCollisionLoader()
{
}

CCollisionMesh::CCollisionOctree* CCollisionLoader::ParseOctree(CInputStream&)
{
    // Not using: Parameter 1 (CInputStream& - src)
    return nullptr;
}

CCollisionMesh::CCollisionOctree::SBranch* CCollisionLoader::ParseOctreeBranch(CInputStream&)
{
    // Not using: Parameter 1 (CInputStream& - src)
    return nullptr;
}

CCollisionMesh::CCollisionOctree::SLeaf* CCollisionLoader::ParseOctreeLeaf(CInputStream&)
{
    // Not using: Parameter 1 (CInputStream& - src)
    return nullptr;
}

void CCollisionLoader::ParseOBBNode(CInputStream& DCLN)
{
    bool b = false;

    while (b == false)
    {
        DCLN.Seek(0x3C, SEEK_CUR);
        b = (DCLN.ReadByte() == 1);
        if (!b) ParseOBBNode(DCLN);
    }

    u32 numFaces = DCLN.ReadLong();
    DCLN.Seek(numFaces * 2, SEEK_CUR);
}

void CCollisionLoader::ReadPropertyFlags(CInputStream& src)
{
    CCollisionMesh::SCollisionProperties property;

    if (mVersion == ePrime)
    {
        u32 flag = src.ReadLong();
        property.Invert = (flag >> 25) & 0x1;
    }

    else if (mVersion == eEchoes)
    {
        u64 flag = src.ReadLongLong();
        property.Invert = (flag >> 24) & 0x1;
    }

    else if (mVersion == eReturns)
    {
        u64 flag = src.ReadLongLong();
        property.Invert = (flag >> 28) & 0x1;
    }

    mProperties.push_back(property);
}

void CCollisionLoader::LoadCollisionIndices(CInputStream &file, bool buildAABox)
{
    // Properties
    u32 propSetCount = file.ReadLong();
    for (u32 iProp = 0; iProp < propSetCount; iProp++)
        ReadPropertyFlags(file);

    // Property indices for vertices/lines/faces
    u32 vtxIndexCount = file.ReadLong();
    std::vector<u8> vtxIndices(vtxIndexCount);
    file.ReadBytes(vtxIndices.data(), vtxIndices.size());

    u32 lineIndexCount = file.ReadLong();
    std::vector<u8> lineIndices(lineIndexCount);
    file.ReadBytes(lineIndices.data(), lineIndices.size());

    u32 faceIndexCount = file.ReadLong();
    std::vector<u8> faceIndices(faceIndexCount);
    file.ReadBytes(faceIndices.data(), faceIndices.size());

    // Lines
    mpMesh->mLineCount = file.ReadLong();
    mpMesh->mCollisionLines.resize(mpMesh->mLineCount);
    for (u32 iLine = 0; iLine < mpMesh->mLineCount; iLine++)
    {
        CCollisionMesh::CCollisionLine *pLine = &mpMesh->mCollisionLines[iLine];
        pLine->Vertices[0] = file.ReadShort();
        pLine->Vertices[1] = file.ReadShort();
        pLine->Properties =  mProperties[lineIndices[iLine]];
    }

    // Faces
    mpMesh->mFaceCount = file.ReadLong() / 3; // Not sure why they store it this way. It's inconsistent.
    mpMesh->mCollisionFaces.resize(mpMesh->mFaceCount);

    for (u32 iFace = 0; iFace < mpMesh->mFaceCount; iFace++)
    {
        CCollisionMesh::CCollisionFace *pFace = &mpMesh->mCollisionFaces[iFace];
        pFace->Lines[0] = file.ReadShort();
        pFace->Lines[1] = file.ReadShort();
        pFace->Lines[2] = file.ReadShort();
        pFace->Properties = mProperties[faceIndices[iFace]];
    }

    // Echoes introduces a new data chunk; don't know what it is yet, skipping for now
    if (mVersion >= eEchoes)
    {
        u32 unknownCount = file.ReadLong();
        file.Seek(unknownCount * 2, SEEK_CUR);
    }

    // Vertices
    mpMesh->mVertexCount = file.ReadLong();
    mpMesh->mCollisionVertices.resize(mpMesh->mVertexCount);
    CAABox bounds;

    for (u32 iVtx = 0; iVtx < mpMesh->mVertexCount; iVtx++)
    {
        CCollisionMesh::CCollisionVertex *pVtx = &mpMesh->mCollisionVertices[iVtx];
        pVtx->Pos = CVector3f(file);
        pVtx->Properties = mProperties[vtxIndices[iVtx]];
        if (buildAABox) bounds.ExpandBounds(pVtx->Pos);
    }
    if (buildAABox) mpMesh->mAABox = bounds;
}

// ************ STATIC ************
CCollisionMeshGroup* CCollisionLoader::LoadAreaCollision(CInputStream& MREA)
{
    if (!MREA.IsValid()) return nullptr;
    CCollisionLoader loader;

    MREA.Seek(0x8, SEEK_CUR);
    u32 deafbabe = MREA.ReadLong();
    if (deafbabe != 0xDEAFBABE)
    {
        Log::FileError(MREA.GetSourceString(), MREA.Tell() - 4, "Invalid collision magic: " + TString::HexString(deafbabe));
        return nullptr;
    }

    loader.mVersion = GetFormatVersion(MREA.ReadLong());

    loader.mpGroup = new CCollisionMeshGroup;
    loader.mpMesh = new CCollisionMesh;

    // Octree - structure is known, but not coding this right now
    loader.mpMesh->mAABox = CAABox(MREA);
    MREA.Seek(0x4, SEEK_CUR);
    u32 octreeSize = MREA.ReadLong();
    MREA.Seek(octreeSize, SEEK_CUR); // Skipping the octree for now
    loader.mpMesh->mOctreeLoaded = false;

    // Read collision indices and return
    loader.LoadCollisionIndices(MREA, false);
    loader.mpGroup->AddMesh(loader.mpMesh);
    return loader.mpGroup;
}

CCollisionMeshGroup* CCollisionLoader::LoadDCLN(CInputStream &DCLN)
{
    if (!DCLN.IsValid()) return nullptr;

    CCollisionLoader loader;
    loader.mpGroup = new CCollisionMeshGroup;

    u32 numMeshes = DCLN.ReadLong();

    for (u32 iMesh = 0; iMesh < numMeshes; iMesh++)
    {
        u32 deafbabe = DCLN.ReadLong();

        if (deafbabe != 0xDEAFBABE)
        {
            Log::FileError(DCLN.GetSourceString(), DCLN.Tell() - 4, "Invalid collision magic: " + TString::HexString(deafbabe));
            loader.mpGroup.Delete();
            return nullptr;
        }

        loader.mVersion = GetFormatVersion(DCLN.ReadLong());

        loader.mpMesh = new CCollisionMesh;
        loader.mpMesh->mOctreeLoaded = false;

        if (loader.mVersion == eReturns)
            loader.mpMesh->mAABox = CAABox(DCLN);

        // Read indices and return
        DCLN.Seek(0x4, SEEK_CUR);
        loader.LoadCollisionIndices(DCLN, loader.mVersion != eReturns);
        loader.mpGroup->AddMesh(loader.mpMesh);

        // Parse OBB tree
        loader.ParseOBBNode(DCLN);
    }
    return loader.mpGroup;
}

EGame CCollisionLoader::GetFormatVersion(u32 version)
{
    switch (version)
    {
    case 0x2: return ePrime;
    case 0x3: return ePrime;
    case 0x4: return eEchoes;
    case 0x5: return eReturns;
    default: return eUnknownVersion;
    }
}
