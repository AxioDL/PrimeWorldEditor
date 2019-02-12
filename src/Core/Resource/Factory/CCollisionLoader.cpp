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

SOBBTreeNode* CCollisionLoader::ParseOBBNode(IInputStream& DCLN)
{
    SOBBTreeNode* pOut = nullptr;

    CTransform4f Transform(DCLN);
    CVector3f Radius(DCLN);
    bool IsLeaf = DCLN.ReadBool();

    if (IsLeaf)
    {
        SOBBTreeLeaf* pLeaf = new SOBBTreeLeaf;
        uint NumTris = DCLN.ReadLong();
        pLeaf->TriangleIndices.resize(NumTris);

        for (uint i=0; i<NumTris; i++)
            pLeaf->TriangleIndices[i] = DCLN.ReadShort();

        pOut = pLeaf;
    }
    else
    {
        SOBBTreeBranch* pBranch = new SOBBTreeBranch;
        pBranch->pLeft = std::unique_ptr<SOBBTreeNode>( ParseOBBNode(DCLN) );
        pBranch->pRight = std::unique_ptr<SOBBTreeNode>( ParseOBBNode(DCLN) );
        pOut = pBranch;
    }

    pOut->Transform = Transform;
    pOut->Radii = Radius;
    return pOut;
}

void CCollisionLoader::LoadCollisionMaterial(IInputStream& Src, CCollisionMaterial& OutMaterial)
{
    uint64 RawFlags = (mVersion <= EGame::Prime ? Src.ReadLong() : Src.ReadLongLong());
    OutMaterial.mRawFlags = RawFlags;

    if (mVersion <= EGame::Prime)
    {
        if (RawFlags & 0x00000001) OutMaterial |= eCF_Unknown;
        if (RawFlags & 0x00000002) OutMaterial |= eCF_Stone;
        if (RawFlags & 0x00000004) OutMaterial |= eCF_Metal;
        if (RawFlags & 0x00000008) OutMaterial |= eCF_Grass;
        if (RawFlags & 0x00000010) OutMaterial |= eCF_Ice;
        if (RawFlags & 0x00000040) OutMaterial |= eCF_MetalGrating;
        if (RawFlags & 0x00000080) OutMaterial |= eCF_Phazon;
        if (RawFlags & 0x00000100) OutMaterial |= eCF_Dirt;
        if (RawFlags & 0x00000200) OutMaterial |= eCF_Lava;
        if (RawFlags & 0x00000800) OutMaterial |= eCF_Snow;
        if (RawFlags & 0x00001000) OutMaterial |= eCF_SlowMud;
        if (RawFlags & 0x00004000) OutMaterial |= eCF_Mud;
        if (RawFlags & 0x00008000) OutMaterial |= eCF_Glass;
        if (RawFlags & 0x00010000) OutMaterial |= eCF_Shield;
        if (RawFlags & 0x00020000) OutMaterial |= eCF_Sand;
        if (RawFlags & 0x00040000) OutMaterial |= eCF_ShootThru;
        if (RawFlags & 0x00200000) OutMaterial |= eCF_CameraThru;
        if (RawFlags & 0x00400000) OutMaterial |= eCF_Wood;
        if (RawFlags & 0x00800000) OutMaterial |= eCF_Organic;
        if (RawFlags & 0x02000000) OutMaterial |= eCF_FlippedTri;
        if (RawFlags & 0x08000000) OutMaterial |= eCF_ScanThru;
        if (RawFlags & 0x10000000) OutMaterial |= eCF_AiWalkThru;
        if (RawFlags & 0x20000000) OutMaterial |= eCF_Ceiling;
        if (RawFlags & 0x40000000) OutMaterial |= eCF_Wall;
        if (RawFlags & 0x80000000) OutMaterial |= eCF_Floor;
    }

    else if (mVersion <= EGame::Corruption)
    {
        if (RawFlags & 0x00000001) OutMaterial |= eCF_Unknown;
        if (RawFlags & 0x00000002) OutMaterial |= eCF_Stone;
        if (RawFlags & 0x00000004) OutMaterial |= eCF_Metal;
        if (RawFlags & 0x00000008) OutMaterial |= eCF_Grass;
        if (RawFlags & 0x00000010) OutMaterial |= eCF_Ice;
        if (RawFlags & 0x00000040) OutMaterial |= eCF_MetalGrating;
        if (RawFlags & 0x00000080) OutMaterial |= eCF_Phazon;
        if (RawFlags & 0x00000100) OutMaterial |= eCF_Dirt;
        if (RawFlags & 0x00000200) OutMaterial |= eCF_AltMetal;
        if (RawFlags & 0x00000400) OutMaterial |= eCF_Glass;
        if (RawFlags & 0x00000800) OutMaterial |= eCF_Snow;
        if (RawFlags & 0x00001000) OutMaterial |= eCF_Fabric;
        if (RawFlags & 0x00010000) OutMaterial |= eCF_Shield;
        if (RawFlags & 0x00020000) OutMaterial |= eCF_Sand;
        if (RawFlags & 0x00040000) OutMaterial |= eCF_MothSeedOrganics;
        if (RawFlags & 0x00080000) OutMaterial |= eCF_Web;
        if (RawFlags & 0x00100000) OutMaterial |= eCF_ShootThru;
        if (RawFlags & 0x00200000) OutMaterial |= eCF_CameraThru;
        if (RawFlags & 0x00400000) OutMaterial |= eCF_Wood;
        if (RawFlags & 0x00800000) OutMaterial |= eCF_Organic;
        if (RawFlags & 0x01000000) OutMaterial |= eCF_FlippedTri;
        if (RawFlags & 0x02000000) OutMaterial |= eCF_Rubber;
        if (RawFlags & 0x08000000) OutMaterial |= eCF_ScanThru;
        if (RawFlags & 0x10000000) OutMaterial |= eCF_AiWalkThru;
        if (RawFlags & 0x20000000) OutMaterial |= eCF_Ceiling;
        if (RawFlags & 0x40000000) OutMaterial |= eCF_Wall;
        if (RawFlags & 0x80000000) OutMaterial |= eCF_Floor;

        if (RawFlags & 0x0001000000000000) OutMaterial |= eCF_AiBlock;
        if (RawFlags & 0x0400000000000000) OutMaterial |= eCF_JumpNotAllowed;
    }

    else if (mVersion == EGame::DKCReturns)
    {
        if (RawFlags & 0x10000000) OutMaterial |= eCF_FlippedTri;
    }
}

void CCollisionLoader::LoadCollisionIndices(IInputStream& File, SCollisionIndexData& OutData)
{
    // Materials
    uint NumMaterials = File.ReadLong();
    OutData.Materials.resize( NumMaterials );

    for (uint i=0; i<NumMaterials; i++)
    {
        LoadCollisionMaterial(File, OutData.Materials[i]);
    }

    // Property indices for vertices/edges/triangles
    uint VertexMaterialCount = File.ReadLong();
    OutData.VertexMaterialIndices.resize(VertexMaterialCount);
    File.ReadBytes(OutData.VertexMaterialIndices.data(), VertexMaterialCount);

    uint32 EdgeMaterialCount = File.ReadLong();
    OutData.EdgeMaterialIndices.resize(EdgeMaterialCount);
    File.ReadBytes(OutData.EdgeMaterialIndices.data(), EdgeMaterialCount);

    uint32 TriMaterialCount = File.ReadLong();
    OutData.TriangleMaterialIndices.resize(TriMaterialCount);
    File.ReadBytes(OutData.TriangleMaterialIndices.data(), TriMaterialCount);

    // Edges
    uint NumEdges = File.ReadLong();
    OutData.EdgeIndices.resize( NumEdges * 2 );

    for (uint i=0; i<OutData.EdgeIndices.size(); i++)
    {
        OutData.EdgeIndices[i] = File.ReadShort();
    }

    // Triangles
    uint NumTris = File.ReadLong();
    OutData.TriangleIndices.resize( NumTris );

    for (uint i=0; i<NumTris; i++)
    {
        OutData.TriangleIndices[i] = File.ReadShort();
    }

    // Echoes introduces a new data chunk; don't know what it is yet, skipping for now
    if (mVersion >= EGame::Echoes)
    {
        uint UnknownCount = File.ReadLong();
        File.Skip(UnknownCount * 2);
    }

    // Vertices
    uint NumVertices = File.ReadLong();
    OutData.Vertices.resize(NumVertices);

    for (uint32 i=0; i<NumVertices; i++)
    {
        OutData.Vertices[i].Read(File);
    }
}

// ************ STATIC ************
CCollisionMeshGroup* CCollisionLoader::LoadAreaCollision(IInputStream& rMREA)
{
    if (!rMREA.IsValid()) return nullptr;
    rMREA.Skip(0x8); // Skipping unknown value + collion section size

    // Validate magic
    uint32 DeafBabe = rMREA.ReadLong();
    if (DeafBabe != 0xDEAFBABE)
    {
        errorf("%s [0x%X]: Invalid collision magic: 0x%08X", *rMREA.GetSourceString(), rMREA.Tell() - 4, DeafBabe);
        return nullptr;
    }

    CCollisionLoader Loader;
    Loader.mVersion = GetFormatVersion(rMREA.ReadLong());
    Loader.mpMesh = new CCollisionMesh;

    // Octree - structure is known, but not coding this right now
    Loader.mpMesh->mAABox = CAABox(rMREA);
    rMREA.Skip(0x4);
    uint32 OctreeSize = rMREA.ReadLong();
    rMREA.Skip(OctreeSize); // Skipping the octree for now

    // Read collision indices and return
    Loader.LoadCollisionIndices(rMREA, Loader.mpMesh->mIndexData);

    CCollisionMeshGroup* pOut = new CCollisionMeshGroup();
    pOut->AddMesh(Loader.mpMesh);
    return pOut;
}

CCollisionMeshGroup* CCollisionLoader::LoadDCLN(IInputStream& rDCLN, CResourceEntry *pEntry)
{
    if (!rDCLN.IsValid()) return nullptr;

    CCollisionLoader Loader;
    Loader.mpGroup = new CCollisionMeshGroup(pEntry);

    uint32 NumMeshes = rDCLN.ReadLong();

    for (uint32 MeshIdx = 0; MeshIdx < NumMeshes; MeshIdx++)
    {
        uint32 DeafBabe = rDCLN.ReadLong();

        if (DeafBabe != 0xDEAFBABE)
        {
            errorf("%s [0x%X]: Invalid collision magic: 0x%08X", *rDCLN.GetSourceString(), rDCLN.Tell() - 4, DeafBabe);
            delete Loader.mpGroup;
            return nullptr;
        }

        Loader.mVersion = GetFormatVersion(rDCLN.ReadLong());

        Loader.mpMesh = new CCollidableOBBTree;

        if (Loader.mVersion == EGame::DKCReturns)
            Loader.mpMesh->mAABox = CAABox(rDCLN);

        // Read indices and return
        rDCLN.Seek(0x4, SEEK_CUR);
        Loader.LoadCollisionIndices(rDCLN, Loader.mpMesh->mIndexData);
        Loader.mpGroup->AddMesh(Loader.mpMesh);

        // Build bounding box
        if (Loader.mVersion != EGame::DKCReturns)
        {
            Loader.mpMesh->mAABox = CAABox::skInfinite;

            for (uint i=0; i<Loader.mpMesh->mIndexData.Vertices.size(); i++)
            {
                Loader.mpMesh->mAABox.ExpandBounds(
                    Loader.mpMesh->mIndexData.Vertices[i]
                );
            }
        }

        // Parse OBB tree
        CCollidableOBBTree* pOBBTree = static_cast<CCollidableOBBTree*>(Loader.mpMesh);
        pOBBTree->mpOBBTree = std::unique_ptr<SOBBTreeNode>( Loader.ParseOBBNode(rDCLN) );
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
