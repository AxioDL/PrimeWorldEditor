#include "CPoiToWorld.h"

CPoiToWorld::CPoiToWorld()
{
}

CPoiToWorld::~CPoiToWorld()
{
}

void CPoiToWorld::LinksForMeshID(std::list<u32>& rOutInstanceIDs, u32 MeshID)
{
    for (u32 iLink = 0; iLink < mMeshLinks.size(); iLink++)
    {
        if (mMeshLinks[iLink].MeshID == MeshID)
            rOutInstanceIDs.push_back(mMeshLinks[iLink].PoiInstanceID);
    }
}

void CPoiToWorld::LinksForInstanceID(std::list<u32>& rOutMeshIDs, u32 InstanceID)
{
    for (u32 iLink = 0; iLink < mMeshLinks.size(); iLink++)
    {
        if (mMeshLinks[iLink].PoiInstanceID == InstanceID)
            rOutMeshIDs.push_back(mMeshLinks[iLink].MeshID);
    }
}
