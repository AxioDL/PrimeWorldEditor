#ifndef CCOLLISIONMESHGROUP_H
#define CCOLLISIONMESHGROUP_H

#include "CResource.h"
#include "CCollisionMesh.h"
#include "TResPtr.h"
#include <vector>

class CCollisionMeshGroup : public CResource
{
    DECLARE_RESOURCE_TYPE(DynamicCollision)
    std::vector<CCollisionMesh*> mMeshes;

public:
    CCollisionMeshGroup(CResourceEntry *pEntry = 0) : CResource(pEntry) {}

    ~CCollisionMeshGroup()
    {
        for (auto it = mMeshes.begin(); it != mMeshes.end(); it++)
            delete *it;
    }

    inline uint32 NumMeshes() const                         { return mMeshes.size(); }
    inline CCollisionMesh* MeshByIndex(uint32 Index) const  { return mMeshes[Index]; }
    inline void AddMesh(CCollisionMesh *pMesh)              { mMeshes.push_back(pMesh); }

    inline void Draw()
    {
        for (auto it = mMeshes.begin(); it != mMeshes.end(); it++)
            (*it)->Draw();
    }

    inline void DrawWireframe()
    {
        for (auto it = mMeshes.begin(); it != mMeshes.end(); it++)
            (*it)->DrawWireframe();
    }
};

#endif // CCOLLISIONMESHGROUP_H
