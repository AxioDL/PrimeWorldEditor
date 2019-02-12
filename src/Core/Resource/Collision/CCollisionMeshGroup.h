#ifndef CCOLLISIONMESHGROUP_H
#define CCOLLISIONMESHGROUP_H

#include "CCollisionMesh.h"
#include "Core/Resource/CResource.h"
#include "Core/Resource/TResPtr.h"
#include <Common/Math/CTransform4f.h>
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

    inline void BuildRenderData()
    {
        for (auto It = mMeshes.begin(); It != mMeshes.end(); It++)
            (*It)->BuildRenderData();
    }

    inline void Draw()
    {
        for (auto it = mMeshes.begin(); it != mMeshes.end(); it++)
            (*it)->GetRenderData().Render(false);
    }

    inline void DrawWireframe()
    {
        for (auto it = mMeshes.begin(); it != mMeshes.end(); it++)
            (*it)->GetRenderData().Render(true);
    }
};

#endif // CCOLLISIONMESHGROUP_H
