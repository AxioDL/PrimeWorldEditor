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

    uint32 NumFaces = rDCLN.ReadLong();
    rDCLN.Seek(NumFaces * 2, SEEK_CUR);
}

void CCollisionLoader::ReadPropertyFlags(IInputStream& rSrc)
{
    CCollisionMaterial Material;
    uint64 RawFlags = (mVersion <= EGame::Prime ? rSrc.ReadLong() : rSrc.ReadLongLong());
    Material.mRawFlags = RawFlags;

    if (mVersion <= EGame::Prime)
    {
        if (RawFlags & 0x00000001) Material |= eCF_Unknown;
        if (RawFlags & 0x00000002) Material |= eCF_Stone;
        if (RawFlags & 0x00000004) Material |= eCF_Metal;
        if (RawFlags & 0x00000008) Material |= eCF_Grass;
        if (RawFlags & 0x00000010) Material |= eCF_Ice;
        if (RawFlags & 0x00000040) Material |= eCF_MetalGrating;
        if (RawFlags & 0x00000080) Material |= eCF_Phazon;
        if (RawFlags & 0x00000100) Material |= eCF_Dirt;
        if (RawFlags & 0x00000200) Material |= eCF_Lava;
        if (RawFlags & 0x00000800) Material |= eCF_Snow;
        if (RawFlags & 0x00001000) Material |= eCF_SlowMud;
        if (RawFlags & 0x00004000) Material |= eCF_Mud;
        if (RawFlags & 0x00008000) Material |= eCF_Glass;
        if (RawFlags & 0x00010000) Material |= eCF_Shield;
        if (RawFlags & 0x00020000) Material |= eCF_Sand;
        if (RawFlags & 0x00040000) Material |= eCF_ShootThru;
        if (RawFlags & 0x00200000) Material |= eCF_CameraThru;
        if (RawFlags & 0x00400000) Material |= eCF_Wood;
        if (RawFlags & 0x00800000) Material |= eCF_Organic;
        if (RawFlags & 0x02000000) Material |= eCF_FlippedTri;
        if (RawFlags & 0x08000000) Material |= eCF_ScanThru;
        if (RawFlags & 0x10000000) Material |= eCF_AiWalkThru;
        if (RawFlags & 0x20000000) Material |= eCF_Ceiling;
        if (RawFlags & 0x40000000) Material |= eCF_Wall;
        if (RawFlags & 0x80000000) Material |= eCF_Floor;
    }

    else if (mVersion <= EGame::Corruption)
    {
        if (RawFlags & 0x00000001) Material |= eCF_Unknown;
        if (RawFlags & 0x00000002) Material |= eCF_Stone;
        if (RawFlags & 0x00000004) Material |= eCF_Metal;
        if (RawFlags & 0x00000008) Material |= eCF_Grass;
        if (RawFlags & 0x00000010) Material |= eCF_Ice;
        if (RawFlags & 0x00000040) Material |= eCF_MetalGrating;
        if (RawFlags & 0x00000080) Material |= eCF_Phazon;
        if (RawFlags & 0x00000100) Material |= eCF_Dirt;
        if (RawFlags & 0x00000200) Material |= eCF_AltMetal;
        if (RawFlags & 0x00000400) Material |= eCF_Glass;
        if (RawFlags & 0x00000800) Material |= eCF_Snow;
        if (RawFlags & 0x00001000) Material |= eCF_Fabric;
        if (RawFlags & 0x00010000) Material |= eCF_Shield;
        if (RawFlags & 0x00020000) Material |= eCF_Sand;
        if (RawFlags & 0x00040000) Material |= eCF_MothSeedOrganics;
        if (RawFlags & 0x00080000) Material |= eCF_Web;
        if (RawFlags & 0x00100000) Material |= eCF_ShootThru;
        if (RawFlags & 0x00200000) Material |= eCF_CameraThru;
        if (RawFlags & 0x00400000) Material |= eCF_Wood;
        if (RawFlags & 0x00800000) Material |= eCF_Organic;
        if (RawFlags & 0x01000000) Material |= eCF_FlippedTri;
        if (RawFlags & 0x02000000) Material |= eCF_Rubber;
        if (RawFlags & 0x08000000) Material |= eCF_ScanThru;
        if (RawFlags & 0x10000000) Material |= eCF_AiWalkThru;
        if (RawFlags & 0x20000000) Material |= eCF_Ceiling;
        if (RawFlags & 0x40000000) Material |= eCF_Wall;
        if (RawFlags & 0x80000000) Material |= eCF_Floor;

        if (RawFlags & 0x0001000000000000) Material |= eCF_AiBlock;
        if (RawFlags & 0x0400000000000000) Material |= eCF_JumpNotAllowed;
    }

    else if (mVersion == EGame::DKCReturns)
    {
        if (RawFlags & 0x10000000) Material |= eCF_FlippedTri;
    }

    mpMesh->mMaterials.push_back(Material);
}

void CCollisionLoader::LoadCollisionIndices(IInputStream &rFile, bool BuildAABox)
{
    // Properties
    uint32 PropSetCount = rFile.ReadLong();
    for (uint32 iProp = 0; iProp < PropSetCount; iProp++)
        ReadPropertyFlags(rFile);

    // Property indices for vertices/lines/faces
    uint32 VtxIndexCount = rFile.ReadLong();
    std::vector<uint8> VtxIndices(VtxIndexCount);
    rFile.ReadBytes(VtxIndices.data(), VtxIndices.size());

    uint32 LineIndexCount = rFile.ReadLong();
    std::vector<uint8> LineIndices(LineIndexCount);
    rFile.ReadBytes(LineIndices.data(), LineIndices.size());

    uint32 FaceIndexCount = rFile.ReadLong();
    std::vector<uint8> FaceIndices(FaceIndexCount);
    rFile.ReadBytes(FaceIndices.data(), FaceIndices.size());

    // Lines
    mpMesh->mLineCount = rFile.ReadLong();
    mpMesh->mCollisionLines.resize(mpMesh->mLineCount);
    for (uint32 iLine = 0; iLine < mpMesh->mLineCount; iLine++)
    {
        CCollisionMesh::CCollisionLine *pLine = &mpMesh->mCollisionLines[iLine];
        pLine->Vertices[0] = rFile.ReadShort();
        pLine->Vertices[1] = rFile.ReadShort();
        pLine->MaterialIdx = LineIndices[iLine];
    }

    // Faces
    mpMesh->mFaceCount = rFile.ReadLong() / 3; // Not sure why they store it this way. It's inconsistent.
    mpMesh->mCollisionFaces.resize(mpMesh->mFaceCount);

    for (uint32 iFace = 0; iFace < mpMesh->mFaceCount; iFace++)
    {
        CCollisionMesh::CCollisionFace *pFace = &mpMesh->mCollisionFaces[iFace];
        pFace->Lines[0] = rFile.ReadShort();
        pFace->Lines[1] = rFile.ReadShort();
        pFace->Lines[2] = rFile.ReadShort();
        pFace->MaterialIdx = FaceIndices[iFace];
    }

    // Echoes introduces a new data chunk; don't know what it is yet, skipping for now
    if (mVersion >= EGame::Echoes)
    {
        uint32 UnknownCount = rFile.ReadLong();
        rFile.Seek(UnknownCount * 2, SEEK_CUR);
    }

    // Vertices
    mpMesh->mVertexCount = rFile.ReadLong();
    mpMesh->mCollisionVertices.resize(mpMesh->mVertexCount);
    CAABox Bounds;

    for (uint32 iVtx = 0; iVtx < mpMesh->mVertexCount; iVtx++)
    {
        CCollisionMesh::CCollisionVertex *pVtx = &mpMesh->mCollisionVertices[iVtx];
        pVtx->Pos = CVector3f(rFile);
        pVtx->MaterialIdx = VtxIndices[iVtx];
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
    uint32 DeafBabe = rMREA.ReadLong();
    if (DeafBabe != 0xDEAFBABE)
    {
        errorf("%s [0x%X]: Invalid collision magic: 0x%08X", *rMREA.GetSourceString(), rMREA.Tell() - 4, DeafBabe);
        return nullptr;
    }

    loader.mVersion = GetFormatVersion(rMREA.ReadLong());

    loader.mpGroup = new CCollisionMeshGroup;
    loader.mpMesh = new CCollisionMesh;

    // Octree - structure is known, but not coding this right now
    loader.mpMesh->mAABox = CAABox(rMREA);
    rMREA.Seek(0x4, SEEK_CUR);
    uint32 OctreeSize = rMREA.ReadLong();
    rMREA.Seek(OctreeSize, SEEK_CUR); // Skipping the octree for now
    loader.mpMesh->mOctreeLoaded = false;

    // Read collision indices and return
    loader.LoadCollisionIndices(rMREA, false);
    loader.mpGroup->AddMesh(loader.mpMesh);
    return loader.mpGroup;
}

CCollisionMeshGroup* CCollisionLoader::LoadDCLN(IInputStream& rDCLN, CResourceEntry *pEntry)
{
    if (!rDCLN.IsValid()) return nullptr;

    CCollisionLoader Loader;
    Loader.mpGroup = new CCollisionMeshGroup(pEntry);

    uint32 NumMeshes = rDCLN.ReadLong();

    for (uint32 iMesh = 0; iMesh < NumMeshes; iMesh++)
    {
        uint32 DeafBabe = rDCLN.ReadLong();

        if (DeafBabe != 0xDEAFBABE)
        {
            errorf("%s [0x%X]: Invalid collision magic: 0x%08X", *rDCLN.GetSourceString(), rDCLN.Tell() - 4, DeafBabe);
            Loader.mpGroup.Delete();
            return nullptr;
        }

        Loader.mVersion = GetFormatVersion(rDCLN.ReadLong());

        Loader.mpMesh = new CCollisionMesh;
        Loader.mpMesh->mOctreeLoaded = false;

        if (Loader.mVersion == EGame::DKCReturns)
            Loader.mpMesh->mAABox = CAABox(rDCLN);

        // Read indices and return
        rDCLN.Seek(0x4, SEEK_CUR);
        Loader.LoadCollisionIndices(rDCLN, Loader.mVersion != EGame::DKCReturns);
        Loader.mpGroup->AddMesh(Loader.mpMesh);

        // Parse OBB tree
        Loader.ParseOBBNode(rDCLN);
    }
    return Loader.mpGroup;
}

EGame CCollisionLoader::GetFormatVersion(uint32 Version)
{
    switch (Version)
    {
    case 0x2: return EGame::Prime;
    case 0x3: return EGame::Prime;
    case 0x4: return EGame::Echoes;
    case 0x5: return EGame::DKCReturns;
    default: return EGame::Invalid;
    }
}
