#ifndef CCOLLISIONMESHGROUP_H
#define CCOLLISIONMESHGROUP_H

#include "CResource.h"
#include "CCollisionMesh.h"
#include <Core/CToken.h>
#include <vector>

class CCollisionMeshGroup : public CResource
{
    std::vector<CCollisionMesh*> mMeshes;

public:
    CCollisionMeshGroup();
    ~CCollisionMeshGroup();
    EResType Type();

    u32 NumMeshes();
    CCollisionMesh* MeshByIndex(u32 index);
    void AddMesh(CCollisionMesh *pMesh);
    void Draw();
    void DrawWireframe();
};

#endif // CCOLLISIONMESHGROUP_H
