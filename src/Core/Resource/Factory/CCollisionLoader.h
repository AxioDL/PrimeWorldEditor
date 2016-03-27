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
    CCollisionMesh::CCollisionOctree* ParseOctree(IInputStream& rSrc);
    CCollisionMesh::CCollisionOctree::SBranch* ParseOctreeBranch(IInputStream& rSrc);
    CCollisionMesh::CCollisionOctree::SLeaf* ParseOctreeLeaf(IInputStream& rSrc);
    void ParseOBBNode(IInputStream& rDCLN);
    void ReadPropertyFlags(IInputStream& rSrc);
    void LoadCollisionIndices(IInputStream& rFile, bool BuildAABox);

public:
    static CCollisionMeshGroup* LoadAreaCollision(IInputStream& rMREA);
    static CCollisionMeshGroup* LoadDCLN(IInputStream& rDCLN);
    static EGame GetFormatVersion(u32 Version);
};

#endif // CCOLLISIONLOADER_H
