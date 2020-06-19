#ifndef CPOITOWORLD_H
#define CPOITOWORLD_H

#include "CResource.h"
#include <list>
#include <map>
#include <memory>
#include <vector>

class CPoiToWorld : public CResource
{
    DECLARE_RESOURCE_TYPE(StaticGeometryMap)

public:
    struct SPoiMap
    {
        uint32 PoiID;
        std::list<uint32> ModelIDs;
    };

private:
    std::vector<std::unique_ptr<SPoiMap>> mMaps;
    std::map<uint32,SPoiMap*> mPoiLookupMap;

public:
    explicit CPoiToWorld(CResourceEntry *pEntry = nullptr);
    ~CPoiToWorld() override;

    void AddPoi(uint32 PoiID);
    void AddPoiMeshMap(uint32 PoiID, uint32 ModelID);
    void RemovePoi(uint32 PoiID);
    void RemovePoiMeshMap(uint32 PoiID, uint32 ModelID);

    size_t NumMappedPOIs() const
    {
        return mMaps.size();
    }

    const SPoiMap* MapByIndex(size_t Index) const
    {
        return mMaps[Index].get();
    }

    const SPoiMap* MapByID(uint32 InstanceID) const
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
