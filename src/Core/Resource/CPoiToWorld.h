#ifndef CPOITOWORLD_H
#define CPOITOWORLD_H

#include "CResource.h"
#include <list>

class CPoiToWorld : public CResource
{
    DECLARE_RESOURCE_TYPE(ePoiToWorld)
    friend class CPoiToWorldLoader;

public:
    struct SPoiMeshLink
    {
        u32 MeshID;
        u32 PoiInstanceID;
    };

private:
    std::vector<SPoiMeshLink> mMeshLinks;

public:
    CPoiToWorld();
    ~CPoiToWorld();

    void LinksForMeshID(std::list<u32>& rOutInstanceIDs, u32 MeshID);
    void LinksForInstanceID(std::list<u32>& rOutMeshIDs, u32 InstanceID);

    inline u32 NumMeshLinks()
    {
        return mMeshLinks.size();
    }

    inline const SPoiMeshLink& MeshLinkByIndex(u32 Index)
    {
        return mMeshLinks[Index];
    }
};

#endif // CPOITOWORLD_H
