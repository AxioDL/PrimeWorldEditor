#ifndef CCOLLISIONLOADER_H
#define CCOLLISIONLOADER_H

#include "../CCollisionMesh.h"

class CCollisionLoader
{
    enum ECollisionVersion;

    CCollisionMesh *mesh;
    ECollisionVersion version;
    std::vector<CCollisionMesh::SCollisionProperties> properties;

    enum ECollisionVersion
    {
        Prime = 0x3,
        Echoes = 0x4,
        DonkeyKongCountryReturns = 0x5
    };

    CCollisionLoader();
    CCollisionMesh::CCollisionOctree* parseOctree(CInputStream& src);
    CCollisionMesh::CCollisionOctree::SBranch* parseOctreeBranch(CInputStream& src);
    CCollisionMesh::CCollisionOctree::SLeaf* parseOctreeLeaf(CInputStream& src);
    void readPropertyFlags(CInputStream& src);

public:
    static CCollisionMesh* LoadAreaCollision(CInputStream& MREA);
};

#endif // CCOLLISIONLOADER_H
