#include "CCollisionLoader.h"
#include <Core/Log.h>
#include <iostream>

CCollisionLoader::CCollisionLoader()
{
}

CCollisionMesh* CCollisionLoader::LoadAreaCollision(CInputStream& MREA)
{
    if (!MREA.IsValid()) return nullptr;
    CCollisionLoader loader;

    MREA.Seek(0x8, SEEK_CUR);
    u32 deafbabe = MREA.ReadLong();
    if (deafbabe != 0xdeafbabe)
    {
        Log::FileError(MREA.GetSourceString(), MREA.Tell() - 4, "Invalid collision magic: " + StringUtil::ToHexString(deafbabe));
        return nullptr;
    }

    u32 version = MREA.ReadLong();
    loader.version = ECollisionVersion(version);
    if ((loader.version != Prime) && (loader.version != Echoes))
    {
        Log::FileError(MREA.GetSourceString(), MREA.Tell() - 4, "Unsupported collision version: " + StringUtil::ToHexString(version));
        return nullptr;
    }

    loader.mesh = new CCollisionMesh;
    CCollisionMesh *cmesh = loader.mesh;

    // Octree - structure is known, but not coding this right now
    cmesh->mAABox = CAABox(MREA);
    MREA.Seek(0x4, SEEK_CUR);
    u32 octreeSize = MREA.ReadLong();
    MREA.Seek(octreeSize, SEEK_CUR); // Skipping the octree for now
    cmesh->mOctreeLoaded = false;

    // Properties
    u32 propertySetCount = MREA.ReadLong();
    for (u32 p = 0; p < propertySetCount; p++)
        loader.readPropertyFlags(MREA);

    // Property indices for vertices/lines/faces
    u32 vtxIndexCount = MREA.ReadLong();
    std::vector<u8> vtxIndices(vtxIndexCount);
    MREA.ReadBytes(vtxIndices.data(), vtxIndices.size());

    u32 lineIndexCount = MREA.ReadLong();
    std::vector<u8> lineIndices(lineIndexCount);
    MREA.ReadBytes(lineIndices.data(), lineIndices.size());

    u32 faceIndexCount = MREA.ReadLong();
    std::vector<u8> faceIndices(faceIndexCount);
    MREA.ReadBytes(faceIndices.data(), faceIndices.size());

    // Lines
    cmesh->mLineCount = MREA.ReadLong();
    cmesh->mCollisionLines.resize(cmesh->mLineCount);
    for (u32 l = 0; l < cmesh->mLineCount; l++)
    {
        CCollisionMesh::CCollisionLine *Line = &cmesh->mCollisionLines[l];
        Line->Vertices[0] = MREA.ReadShort();
        Line->Vertices[1] = MREA.ReadShort();
        Line->Properties =  loader.properties[lineIndices[l]];
    }

    // Faces
    cmesh->mFaceCount = MREA.ReadLong() / 3; // Not sure why they store it this way. It's inconsistent.
    cmesh->mCollisionFaces.resize(cmesh->mFaceCount);
    for (u32 f = 0; f < cmesh->mFaceCount; f++)
    {
        CCollisionMesh::CCollisionFace *face = &cmesh->mCollisionFaces[f];
        face->Lines[0] = MREA.ReadShort();
        face->Lines[1] = MREA.ReadShort();
        face->Lines[2] = MREA.ReadShort();
        face->Properties = loader.properties[faceIndices[f]];
    }

    // Echoes introduces a new data chunk; don't know what it is yet, skipping for now
    if (loader.version == Echoes)
    {
        u32 unknown_count = MREA.ReadLong();
        MREA.Seek(unknown_count * 2, SEEK_CUR);
    }

    // Vertices
    cmesh->mVertexCount = MREA.ReadLong();
    cmesh->mCollisionVertices.resize(cmesh->mVertexCount);
    for (u32 v = 0; v < cmesh->mVertexCount; v++)
    {
        CCollisionMesh::CCollisionVertex *vtx = &cmesh->mCollisionVertices[v];
        vtx->Pos = CVector3f(MREA);
        vtx->Properties = loader.properties[vtxIndices[v]];
    }

    return cmesh;
}

CCollisionMesh::CCollisionOctree* CCollisionLoader::parseOctree(CInputStream&)
{
    // Not using: Parameter 1 (CInputStream& - src)
    return nullptr;
}

CCollisionMesh::CCollisionOctree::SBranch* CCollisionLoader::parseOctreeBranch(CInputStream&)
{
    // Not using: Parameter 1 (CInputStream& - src)
    return nullptr;
}

CCollisionMesh::CCollisionOctree::SLeaf* CCollisionLoader::parseOctreeLeaf(CInputStream&)
{
    // Not using: Parameter 1 (CInputStream& - src)
    return nullptr;
}

void CCollisionLoader::readPropertyFlags(CInputStream& src)
{
    CCollisionMesh::SCollisionProperties property;

    if (version == Prime)
    {
        u32 flag = src.ReadLong();
        property.Invert = (flag >> 25) & 0x1;
    }

    if (version == Echoes)
    {
        u64 flag = src.ReadLongLong();
        property.Invert = (flag >> 24) & 0x1;
    }

    properties.push_back(property);
}
