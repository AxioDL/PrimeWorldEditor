#ifndef CCOLLISIONLOADER_H
#define CCOLLISIONLOADER_H

#include "Core/Resource/CCollisionMesh.h"
#include "Core/Resource/CCollisionMeshGroup.h"
#include "Core/Resource/EFormatVersion.h"

class CCollisionLoader
{
    TResPtr<CCollisionMeshGroup> mpGroup;
    CCollisionMesh *mpMesh;
    EGame mVersion;
    std::vector<CCollisionMesh::SCollisionProperties> mProperties;

    CCollisionLoader();
    CCollisionMesh::CCollisionOctree* ParseOctree(CInputStream& src);
    CCollisionMesh::CCollisionOctree::SBranch* ParseOctreeBranch(CInputStream& src);
    CCollisionMesh::CCollisionOctree::SLeaf* ParseOctreeLeaf(CInputStream& src);
    void ParseOBBNode(CInputStream& DCLN);
    void ReadPropertyFlags(CInputStream& src);
    void LoadCollisionIndices(CInputStream& file, bool buildAABox);

public:
    static CCollisionMeshGroup* LoadAreaCollision(CInputStream& MREA);
    static CCollisionMeshGroup* LoadDCLN(CInputStream& DCLN);
    static EGame GetFormatVersion(u32 version);
};

#endif // CCOLLISIONLOADER_H
