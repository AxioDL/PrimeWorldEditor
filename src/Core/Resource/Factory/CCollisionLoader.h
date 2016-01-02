#ifndef CCOLLISIONLOADER_H
#define CCOLLISIONLOADER_H

#include "Core/Resource/CCollisionMesh.h"
#include "Core/Resource/CCollisionMeshGroup.h"
#include "Core/Resource/EGame.h"

class CCollisionLoader
{
    TResPtr<CCollisionMeshGroup> mpGroup;
    CCollisionMesh *mpMesh;
    EGame mVersion;
    std::vector<CCollisionMesh::SCollisionProperties> mProperties;

    CCollisionLoader();
    CCollisionMesh::CCollisionOctree* ParseOctree(IInputStream& src);
    CCollisionMesh::CCollisionOctree::SBranch* ParseOctreeBranch(IInputStream& src);
    CCollisionMesh::CCollisionOctree::SLeaf* ParseOctreeLeaf(IInputStream& src);
    void ParseOBBNode(IInputStream& DCLN);
    void ReadPropertyFlags(IInputStream& src);
    void LoadCollisionIndices(IInputStream& file, bool buildAABox);

public:
    static CCollisionMeshGroup* LoadAreaCollision(IInputStream& MREA);
    static CCollisionMeshGroup* LoadDCLN(IInputStream& DCLN);
    static EGame GetFormatVersion(u32 version);
};

#endif // CCOLLISIONLOADER_H
