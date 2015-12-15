#include "CCollisionMeshGroup.h"

CCollisionMeshGroup::CCollisionMeshGroup()
{
}

CCollisionMeshGroup::~CCollisionMeshGroup()
{
    for (auto it = mMeshes.begin(); it != mMeshes.end(); it++)
        delete *it;
}

u32 CCollisionMeshGroup::NumMeshes()
{
    return mMeshes.size();
}

CCollisionMesh* CCollisionMeshGroup::MeshByIndex(u32 index)
{
    return mMeshes[index];
}

void CCollisionMeshGroup::AddMesh(CCollisionMesh *pMesh)
{
    mMeshes.push_back(pMesh);
}

void CCollisionMeshGroup::Draw()
{
    for (auto it = mMeshes.begin(); it != mMeshes.end(); it++)
        (*it)->Draw();
}

void CCollisionMeshGroup::DrawWireframe()
{
    for (auto it = mMeshes.begin(); it != mMeshes.end(); it++)
        (*it)->DrawWireframe();
}
