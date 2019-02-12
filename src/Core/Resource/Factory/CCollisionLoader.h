#ifndef CCOLLISIONLOADER_H
#define CCOLLISIONLOADER_H

#include "Core/Resource/Collision/CCollisionMesh.h"
#include "Core/Resource/Collision/CCollisionMeshGroup.h"
#include "Core/Resource/Collision/CCollidableOBBTree.h"
#include <Common/EGame.h>

class CCollisionLoader
{
    TResPtr<CCollisionMeshGroup> mpGroup;
    CCollisionMesh *mpMesh;
    EGame mVersion;

    CCollisionLoader();

#if 0
    CCollisionMesh::CCollisionOctree* ParseOctree(IInputStream& rSrc);
    CCollisionMesh::CCollisionOctree::SBranch* ParseOctreeBranch(IInputStream& rSrc);
    CCollisionMesh::CCollisionOctree::SLeaf* ParseOctreeLeaf(IInputStream& rSrc);
#endif

    SOBBTreeNode*   ParseOBBNode(IInputStream& DCLN);
    void            LoadCollisionMaterial(IInputStream& Src, CCollisionMaterial& OutMaterial);
    void            LoadCollisionIndices(IInputStream& File, SCollisionIndexData& OutData);

public:
    static CCollisionMeshGroup* LoadAreaCollision(IInputStream& rMREA);
    static CCollisionMeshGroup* LoadDCLN(IInputStream& rDCLN, CResourceEntry *pEntry);
    static EGame                GetFormatVersion(uint32 Version);
};

#endif // CCOLLISIONLOADER_H
