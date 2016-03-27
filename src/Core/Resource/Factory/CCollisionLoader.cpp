#include "CCollisionLoader.h"
#include <Common/Log.h>
#include <iostream>

CCollisionLoader::CCollisionLoader()
{
}

CCollisionMesh::CCollisionOctree* CCollisionLoader::ParseOctree(IInputStream& /*rSrc*/)
{
    return nullptr;
}

CCollisionMesh::CCollisionOctree::SBranch* CCollisionLoader::ParseOctreeBranch(IInputStream& /*rSrc*/)
{
    return nullptr;
}

CCollisionMesh::CCollisionOctree::SLeaf* CCollisionLoader::ParseOctreeLeaf(IInputStream& /*rSrc*/)
{
    return nullptr;
}

void CCollisionLoader::ParseOBBNode(IInputStream& rDCLN)
{
    bool b = false;

    while (b == false)
    {
        rDCLN.Seek(0x3C, SEEK_CUR);
        b = (rDCLN.ReadByte() == 1);
        if (!b) ParseOBBNode(rDCLN);
    }

    u32 NumFaces = rDCLN.ReadLong();
    rDCLN.Seek(NumFaces * 2, SEEK_CUR);
}

void CCollisionLoader::ReadPropertyFlags(IInputStream& rSrc)
{
    CCollisionMesh::SCollisionProperties Property;

    if (mVersion == ePrime)
    {
        u32 Flag = rSrc.ReadLong();
        Property.Invert = (Flag >> 25) & 0x1;
    }

    else if (mVersion == eEchoes)
    {
        u64 Flag = rSrc.ReadLongLong();
        Property.Invert = (Flag >> 24) & 0x1;
    }

    else if (mVersion == eReturns)
    {
        u64 Flag = rSrc.ReadLongLong();
        Property.Invert = (Flag >> 28) & 0x1;
    }

    mProperties.push_back(Property);
}

void CCollisionLoader::LoadCollisionIndices(IInputStream &rFile, bool BuildAABox)
{
    // Properties
    u32 PropSetCount = rFile.ReadLong();
    for (u32 iProp = 0; iProp < PropSetCount; iProp++)
        ReadPropertyFlags(rFile);

    // Property indices for vertices/lines/faces
    u32 VtxIndexCount = rFile.ReadLong();
    std::vector<u8> VtxIndices(VtxIndexCount);
    rFile.ReadBytes(VtxIndices.data(), VtxIndices.size());

    u32 LineIndexCount = rFile.ReadLong();
    std::vector<u8> LineIndices(LineIndexCount);
    rFile.ReadBytes(LineIndices.data(), LineIndices.size());

    u32 FaceIndexCount = rFile.ReadLong();
    std::vector<u8> FaceIndices(FaceIndexCount);
    rFile.ReadBytes(FaceIndices.data(), FaceIndices.size());

    // Lines
    mpMesh->mLineCount = rFile.ReadLong();
    mpMesh->mCollisionLines.resize(mpMesh->mLineCount);
    for (u32 iLine = 0; iLine < mpMesh->mLineCount; iLine++)
    {
        CCollisionMesh::CCollisionLine *pLine = &mpMesh->mCollisionLines[iLine];
        pLine->Vertices[0] = rFile.ReadShort();
        pLine->Vertices[1] = rFile.ReadShort();
        pLine->Properties =  mProperties[LineIndices[iLine]];
    }

    // Faces
    mpMesh->mFaceCount = rFile.ReadLong() / 3; // Not sure why they store it this way. It's inconsistent.
    mpMesh->mCollisionFaces.resize(mpMesh->mFaceCount);

    for (u32 iFace = 0; iFace < mpMesh->mFaceCount; iFace++)
    {
        CCollisionMesh::CCollisionFace *pFace = &mpMesh->mCollisionFaces[iFace];
        pFace->Lines[0] = rFile.ReadShort();
        pFace->Lines[1] = rFile.ReadShort();
        pFace->Lines[2] = rFile.ReadShort();
        pFace->Properties = mProperties[FaceIndices[iFace]];
    }

    // Echoes introduces a new data chunk; don't know what it is yet, skipping for now
    if (mVersion >= eEchoes)
    {
        u32 UnknownCount = rFile.ReadLong();
        rFile.Seek(UnknownCount * 2, SEEK_CUR);
    }

    // Vertices
    mpMesh->mVertexCount = rFile.ReadLong();
    mpMesh->mCollisionVertices.resize(mpMesh->mVertexCount);
    CAABox Bounds;

    for (u32 iVtx = 0; iVtx < mpMesh->mVertexCount; iVtx++)
    {
        CCollisionMesh::CCollisionVertex *pVtx = &mpMesh->mCollisionVertices[iVtx];
        pVtx->Pos = CVector3f(rFile);
        pVtx->Properties = mProperties[VtxIndices[iVtx]];
        if (BuildAABox) Bounds.ExpandBounds(pVtx->Pos);
    }
    if (BuildAABox) mpMesh->mAABox = Bounds;
}

// ************ STATIC ************
CCollisionMeshGroup* CCollisionLoader::LoadAreaCollision(IInputStream& rMREA)
{
    if (!rMREA.IsValid()) return nullptr;
    CCollisionLoader loader;

    rMREA.Seek(0x8, SEEK_CUR);
    u32 DeafBabe = rMREA.ReadLong();
    if (DeafBabe != 0xDEAFBABE)
    {
        Log::FileError(rMREA.GetSourceString(), rMREA.Tell() - 4, "Invalid collision magic: " + TString::HexString(DeafBabe));
        return nullptr;
    }

    loader.mVersion = GetFormatVersion(rMREA.ReadLong());

    loader.mpGroup = new CCollisionMeshGroup;
    loader.mpMesh = new CCollisionMesh;

    // Octree - structure is known, but not coding this right now
    loader.mpMesh->mAABox = CAABox(rMREA);
    rMREA.Seek(0x4, SEEK_CUR);
    u32 OctreeSize = rMREA.ReadLong();
    rMREA.Seek(OctreeSize, SEEK_CUR); // Skipping the octree for now
    loader.mpMesh->mOctreeLoaded = false;

    // Read collision indices and return
    loader.LoadCollisionIndices(rMREA, false);
    loader.mpGroup->AddMesh(loader.mpMesh);
    return loader.mpGroup;
}

CCollisionMeshGroup* CCollisionLoader::LoadDCLN(IInputStream& rDCLN)
{
    if (!rDCLN.IsValid()) return nullptr;

    CCollisionLoader Loader;
    Loader.mpGroup = new CCollisionMeshGroup;

    u32 NumMeshes = rDCLN.ReadLong();

    for (u32 iMesh = 0; iMesh < NumMeshes; iMesh++)
    {
        u32 DeafBabe = rDCLN.ReadLong();

        if (DeafBabe != 0xDEAFBABE)
        {
            Log::FileError(rDCLN.GetSourceString(), rDCLN.Tell() - 4, "Invalid collision magic: " + TString::HexString(DeafBabe));
            Loader.mpGroup.Delete();
            return nullptr;
        }

        Loader.mVersion = GetFormatVersion(rDCLN.ReadLong());

        Loader.mpMesh = new CCollisionMesh;
        Loader.mpMesh->mOctreeLoaded = false;

        if (Loader.mVersion == eReturns)
            Loader.mpMesh->mAABox = CAABox(rDCLN);

        // Read indices and return
        rDCLN.Seek(0x4, SEEK_CUR);
        Loader.LoadCollisionIndices(rDCLN, Loader.mVersion != eReturns);
        Loader.mpGroup->AddMesh(Loader.mpMesh);

        // Parse OBB tree
        Loader.ParseOBBNode(rDCLN);
    }
    return Loader.mpGroup;
}

EGame CCollisionLoader::GetFormatVersion(u32 Version)
{
    switch (Version)
    {
    case 0x2: return ePrime;
    case 0x3: return ePrime;
    case 0x4: return eEchoes;
    case 0x5: return eReturns;
    default: return eUnknownVersion;
    }
}
