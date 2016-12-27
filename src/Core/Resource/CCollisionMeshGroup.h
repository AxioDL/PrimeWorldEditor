#ifndef CCOLLISIONMESHGROUP_H
#define CCOLLISIONMESHGROUP_H

#include "CResource.h"
#include "CCollisionMesh.h"
#include "EGame.h"
#include "TResPtr.h"
#include <vector>

class CCollisionMeshGroup : public CResource
{
    DECLARE_RESOURCE_TYPE(eCollisionMeshGroup)
    std::vector<CCollisionMesh*> mMeshes;
    EGame mGame;

public:
    CCollisionMeshGroup() {}

    ~CCollisionMeshGroup()
    {
        for (auto it = mMeshes.begin(); it != mMeshes.end(); it++)
            delete *it;
    }

    inline EGame Game() const                           { return mGame; }
    inline u32 NumMeshes() const                        { return mMeshes.size(); }
    inline CCollisionMesh* MeshByIndex(u32 Index) const { return mMeshes[Index]; }

    inline void SetGame(EGame Game)                     { mGame = Game; }
    inline void AddMesh(CCollisionMesh *pMesh)          { mMeshes.push_back(pMesh); }

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
