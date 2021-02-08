#include "CCollisionLoader.h"
#include <Common/Log.h>
#include <iostream>

CCollisionLoader::CCollisionLoader()
{
}

#if 0
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
#endif

std::unique_ptr<SOBBTreeNode> CCollisionLoader::ParseOBBNode(IInputStream& DCLN) const
{
    std::unique_ptr<SOBBTreeNode> pOut;

    const CTransform4f Transform(DCLN);
    const CVector3f Radius(DCLN);
    const bool IsLeaf = DCLN.ReadBool();

    if (IsLeaf)
    {
        auto pLeaf = std::make_unique<SOBBTreeLeaf>();
        const uint32 NumTris = DCLN.ReadULong();
        pLeaf->TriangleIndices.resize(NumTris);

        for (auto& index : pLeaf->TriangleIndices)
            index = DCLN.ReadShort();

        pOut = std::move(pLeaf);
    }
    else
    {
        auto pBranch = std::make_unique<SOBBTreeBranch>();
        pBranch->pLeft = ParseOBBNode(DCLN);
        pBranch->pRight = ParseOBBNode(DCLN);
        pOut = std::move(pBranch);
    }

    pOut->Transform = Transform;
    pOut->Radii = Radius;
    return pOut;
}

void CCollisionLoader::LoadCollisionMaterial(IInputStream& Src, CCollisionMaterial& OutMaterial)
{
    const uint64 RawFlags = mVersion <= EGame::Prime ? Src.ReadULong() : Src.ReadULongLong();
    OutMaterial.mRawFlags = RawFlags;

    if (mVersion <= EGame::Prime)
    {
        if ((RawFlags & 0x00000001) != 0) OutMaterial |= eCF_Unknown;
        if ((RawFlags & 0x00000002) != 0) OutMaterial |= eCF_Stone;
        if ((RawFlags & 0x00000004) != 0) OutMaterial |= eCF_Metal;
        if ((RawFlags & 0x00000008) != 0) OutMaterial |= eCF_Grass;
        if ((RawFlags & 0x00000010) != 0) OutMaterial |= eCF_Ice;
        if ((RawFlags & 0x00000040) != 0) OutMaterial |= eCF_MetalGrating;
        if ((RawFlags & 0x00000080) != 0) OutMaterial |= eCF_Phazon;
        if ((RawFlags & 0x00000100) != 0) OutMaterial |= eCF_Dirt;
        if ((RawFlags & 0x00000200) != 0) OutMaterial |= eCF_Lava;
        if ((RawFlags & 0x00000800) != 0) OutMaterial |= eCF_Snow;
        if ((RawFlags & 0x00001000) != 0) OutMaterial |= eCF_SlowMud;
        if ((RawFlags & 0x00004000) != 0) OutMaterial |= eCF_Mud;
        if ((RawFlags & 0x00008000) != 0) OutMaterial |= eCF_Glass;
        if ((RawFlags & 0x00010000) != 0) OutMaterial |= eCF_Shield;
        if ((RawFlags & 0x00020000) != 0) OutMaterial |= eCF_Sand;
        if ((RawFlags & 0x00040000) != 0) OutMaterial |= eCF_ShootThru;
        if ((RawFlags & 0x00200000) != 0) OutMaterial |= eCF_CameraThru;
        if ((RawFlags & 0x00400000) != 0) OutMaterial |= eCF_Wood;
        if ((RawFlags & 0x00800000) != 0) OutMaterial |= eCF_Organic;
        if ((RawFlags & 0x02000000) != 0) OutMaterial |= eCF_FlippedTri;
        if ((RawFlags & 0x08000000) != 0) OutMaterial |= eCF_ScanThru;
        if ((RawFlags & 0x10000000) != 0) OutMaterial |= eCF_AiWalkThru;
        if ((RawFlags & 0x20000000) != 0) OutMaterial |= eCF_Ceiling;
        if ((RawFlags & 0x40000000) != 0) OutMaterial |= eCF_Wall;
        if ((RawFlags & 0x80000000) != 0) OutMaterial |= eCF_Floor;
    }
    else if (mVersion <= EGame::Corruption)
    {
        if ((RawFlags & 0x00000001) != 0) OutMaterial |= eCF_Unknown;
        if ((RawFlags & 0x00000002) != 0) OutMaterial |= eCF_Stone;
        if ((RawFlags & 0x00000004) != 0) OutMaterial |= eCF_Metal;
        if ((RawFlags & 0x00000008) != 0) OutMaterial |= eCF_Grass;
        if ((RawFlags & 0x00000010) != 0) OutMaterial |= eCF_Ice;
        if ((RawFlags & 0x00000040) != 0) OutMaterial |= eCF_MetalGrating;
        if ((RawFlags & 0x00000080) != 0) OutMaterial |= eCF_Phazon;
        if ((RawFlags & 0x00000100) != 0) OutMaterial |= eCF_Dirt;
        if ((RawFlags & 0x00000200) != 0) OutMaterial |= eCF_AltMetal;
        if ((RawFlags & 0x00000400) != 0) OutMaterial |= eCF_Glass;
        if ((RawFlags & 0x00000800) != 0) OutMaterial |= eCF_Snow;
        if ((RawFlags & 0x00001000) != 0) OutMaterial |= eCF_Fabric;
        if ((RawFlags & 0x00010000) != 0) OutMaterial |= eCF_Shield;
        if ((RawFlags & 0x00020000) != 0) OutMaterial |= eCF_Sand;
        if ((RawFlags & 0x00040000) != 0) OutMaterial |= eCF_MothSeedOrganics;
        if ((RawFlags & 0x00080000) != 0) OutMaterial |= eCF_Web;
        if ((RawFlags & 0x00100000) != 0) OutMaterial |= eCF_ShootThru;
        if ((RawFlags & 0x00200000) != 0) OutMaterial |= eCF_CameraThru;
        if ((RawFlags & 0x00400000) != 0) OutMaterial |= eCF_Wood;
        if ((RawFlags & 0x00800000) != 0) OutMaterial |= eCF_Organic;
        if ((RawFlags & 0x01000000) != 0) OutMaterial |= eCF_FlippedTri;
        if ((RawFlags & 0x02000000) != 0) OutMaterial |= eCF_Rubber;
        if ((RawFlags & 0x08000000) != 0) OutMaterial |= eCF_ScanThru;
        if ((RawFlags & 0x10000000) != 0) OutMaterial |= eCF_AiWalkThru;
        if ((RawFlags & 0x20000000) != 0) OutMaterial |= eCF_Ceiling;
        if ((RawFlags & 0x40000000) != 0) OutMaterial |= eCF_Wall;
        if ((RawFlags & 0x80000000) != 0) OutMaterial |= eCF_Floor;

        if ((RawFlags & 0x0001000000000000) != 0) OutMaterial |= eCF_AiBlock;
        if ((RawFlags & 0x0400000000000000) != 0) OutMaterial |= eCF_JumpNotAllowed;
    }
    else if (mVersion == EGame::DKCReturns)
    {
        if ((RawFlags & 0x10000000) != 0) OutMaterial |= eCF_FlippedTri;
    }
}

void CCollisionLoader::LoadCollisionIndices(IInputStream& File, SCollisionIndexData& OutData)
{
    // Materials
    const uint32 NumMaterials = File.ReadULong();
    OutData.Materials.resize(NumMaterials);

    for (auto& material : OutData.Materials)
    {
        LoadCollisionMaterial(File, material);
    }

    // Property indices for vertices/edges/triangles
    const uint32 VertexMaterialCount = File.ReadULong();
    OutData.VertexMaterialIndices.resize(VertexMaterialCount);
    File.ReadBytes(OutData.VertexMaterialIndices.data(), VertexMaterialCount);

    const uint32 EdgeMaterialCount = File.ReadULong();
    OutData.EdgeMaterialIndices.resize(EdgeMaterialCount);
    File.ReadBytes(OutData.EdgeMaterialIndices.data(), EdgeMaterialCount);

    const uint32 TriMaterialCount = File.ReadULong();
    OutData.TriangleMaterialIndices.resize(TriMaterialCount);
    File.ReadBytes(OutData.TriangleMaterialIndices.data(), TriMaterialCount);

    // Edges
    const uint32 NumEdges = File.ReadULong();
    OutData.EdgeIndices.resize(NumEdges * 2);

    for (auto& edge : OutData.EdgeIndices)
    {
        edge = File.ReadUShort();
    }

    // Triangles
    const uint32 NumTris = File.ReadULong();
    OutData.TriangleIndices.resize(NumTris);

    for (auto& triangle : OutData.TriangleIndices)
    {
        triangle = File.ReadUShort();
    }

    // Echoes introduces a new data chunk; don't know what it is yet, skipping for now
    if (mVersion >= EGame::Echoes)
    {
        const uint32 UnknownCount = File.ReadULong();
        File.Skip(UnknownCount * 2);
    }

    // Vertices
    const uint32 NumVertices = File.ReadULong();
    OutData.Vertices.resize(NumVertices);

    for (auto& vert : OutData.Vertices)
    {
        vert.Read(File);
    }
}

// ************ STATIC ************
std::unique_ptr<CCollisionMeshGroup> CCollisionLoader::LoadAreaCollision(IInputStream& rMREA)
{
    if (!rMREA.IsValid())
        return nullptr;

    rMREA.Skip(0x8); // Skipping unknown value + collion section size

    // Validate magic
    const uint32 DeafBabe = rMREA.ReadULong();
    if (DeafBabe != 0xDEAFBABE)
    {
        errorf("%s [0x%X]: Invalid collision magic: 0x%08X", *rMREA.GetSourceString(), rMREA.Tell() - 4, DeafBabe);
        return nullptr;
    }

    auto mesh = std::make_unique<CCollisionMesh>();

    CCollisionLoader Loader;
    Loader.mVersion = GetFormatVersion(rMREA.ReadULong());
    Loader.mpMesh = mesh.get();

    // Octree - structure is known, but not coding this right now
    Loader.mpMesh->mAABox = CAABox(rMREA);
    rMREA.Skip(0x4);
    const uint32 OctreeSize = rMREA.ReadULong();
    rMREA.Skip(OctreeSize); // Skipping the octree for now

    // Read collision indices and return
    Loader.LoadCollisionIndices(rMREA, Loader.mpMesh->mIndexData);

    auto pOut = std::make_unique<CCollisionMeshGroup>();
    pOut->AddMesh(std::move(mesh));
    return pOut;
}

std::unique_ptr<CCollisionMeshGroup> CCollisionLoader::LoadDCLN(IInputStream& rDCLN, CResourceEntry *pEntry)
{
    if (!rDCLN.IsValid()) return nullptr;

    auto ptr = std::make_unique<CCollisionMeshGroup>(pEntry);

    CCollisionLoader Loader;
    Loader.mpGroup = ptr.get();

    const uint32 NumMeshes = rDCLN.ReadULong();

    for (uint32 MeshIdx = 0; MeshIdx < NumMeshes; MeshIdx++)
    {
        const uint32 DeafBabe = rDCLN.ReadULong();

        if (DeafBabe != 0xDEAFBABE)
        {
            errorf("%s [0x%X]: Invalid collision magic: 0x%08X", *rDCLN.GetSourceString(), rDCLN.Tell() - 4, DeafBabe);
            return nullptr;
        }

        auto mesh = std::make_unique<CCollidableOBBTree>();
        Loader.mVersion = GetFormatVersion(rDCLN.ReadULong());
        Loader.mpMesh = mesh.get();

        if (Loader.mVersion == EGame::DKCReturns)
            Loader.mpMesh->mAABox = CAABox(rDCLN);

        // Read indices and return
        rDCLN.Seek(0x4, SEEK_CUR);
        Loader.LoadCollisionIndices(rDCLN, Loader.mpMesh->mIndexData);
        Loader.mpGroup->AddMesh(std::move(mesh));

        // Build bounding box
        if (Loader.mVersion != EGame::DKCReturns)
        {
            Loader.mpMesh->mAABox = CAABox::Infinite();

            for (const auto& vert : Loader.mpMesh->mIndexData.Vertices)
            {
                Loader.mpMesh->mAABox.ExpandBounds(vert);
            }
        }

        // Parse OBB tree
        auto* pOBBTree = static_cast<CCollidableOBBTree*>(Loader.mpMesh);
        pOBBTree->mpOBBTree = Loader.ParseOBBNode(rDCLN);
    }

    return ptr;
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
