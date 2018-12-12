#ifndef CPOITOWORLD_H
#define CPOITOWORLD_H

#include "CResource.h"
#include <list>
#include <map>
#include <vector>

class CPoiToWorld : public CResource
{
    DECLARE_RESOURCE_TYPE(eStaticGeometryMap)

public:
    struct SPoiMap
    {
        uint32 PoiID;
        std::list<uint32> ModelIDs;
    };

private:
    std::vector<SPoiMap*> mMaps;
    std::map<uint32,SPoiMap*> mPoiLookupMap;

public:
    CPoiToWorld(CResourceEntry *pEntry = 0);
    ~CPoiToWorld();

    void AddPoi(uint32 PoiID);
    void AddPoiMeshMap(uint32 PoiID, uint32 ModelID);
    void RemovePoi(uint32 PoiID);
    void RemovePoiMeshMap(uint32 PoiID, uint32 ModelID);

    inline uint32 NumMappedPOIs() const
    {
        return mMaps.size();
    }

    inline const SPoiMap* MapByIndex(uint32 Index) const
    {
        return mMaps[Index];
    }

    inline const SPoiMap* MapByID(uint32 InstanceID) const
    {
        auto it = mPoiLookupMap.find(InstanceID);

        if (it != mPoiLookupMap.end())
            return it->second;
        else
            return nullptr;
    }

    bool HasPoiMappings(uint32 InstanceID) const
    {
        auto it = mPoiLookupMap.find(InstanceID);
        return (it != mPoiLookupMap.end());
    }
};

#endif // CPOITOWORLD_H
