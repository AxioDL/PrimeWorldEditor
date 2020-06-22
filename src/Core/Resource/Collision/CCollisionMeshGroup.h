#ifndef CCOLLISIONMESHGROUP_H
#define CCOLLISIONMESHGROUP_H

#include "CCollisionMesh.h"
#include "Core/Resource/CResource.h"
#include "Core/Resource/TResPtr.h"
#include <Common/Math/CTransform4f.h>
#include <memory>
#include <vector>

class CCollisionMeshGroup : public CResource
{
    DECLARE_RESOURCE_TYPE(DynamicCollision)
    std::vector<std::unique_ptr<CCollisionMesh>> mMeshes;

public:
    explicit CCollisionMeshGroup(CResourceEntry *pEntry = nullptr) : CResource(pEntry) {}
    ~CCollisionMeshGroup() override = default;

    size_t NumMeshes() const                              { return mMeshes.size(); }
    CCollisionMesh* MeshByIndex(size_t Index) const       { return mMeshes[Index].get(); }
    void AddMesh(std::unique_ptr<CCollisionMesh>&& pMesh) { mMeshes.push_back(std::move(pMesh)); }

    void BuildRenderData()
    {
        for (auto& mesh : mMeshes)
            mesh->BuildRenderData();
    }

    void Draw()
    {
        for (auto& mesh : mMeshes)
            mesh->GetRenderData().Render(false);
    }

    void DrawWireframe()
    {
        for (auto& mesh : mMeshes)
            mesh->GetRenderData().Render(true);
    }
};

#endif // CCOLLISIONMESHGROUP_H
