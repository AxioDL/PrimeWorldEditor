#ifndef CPOITOWORLD_H
#define CPOITOWORLD_H

#include "CResource.h"
#include <list>
#include <map>
#include <vector>

class CPoiToWorld : public CResource
{
    DECLARE_RESOURCE_TYPE(ePoiToWorld)

public:
    struct SPoiMap
    {
        u32 PoiID;
        std::list<u32> ModelIDs;
    };

private:
    std::vector<SPoiMap*> mMaps;
    std::map<u32,SPoiMap*> mPoiLookupMap;

public:
    CPoiToWorld();
    ~CPoiToWorld();

    void AddPoi(u32 PoiID);
    void AddPoiMeshMap(u32 PoiID, u32 ModelID);
    void RemovePoi(u32 PoiID);
    void RemovePoiMeshMap(u32 PoiID, u32 ModelID);

    inline u32 NumMappedPOIs() const
    {
        return mMaps.size();
    }

    inline const SPoiMap* MapByIndex(u32 Index) const
    {
        return mMaps[Index];
    }
};

#endif // CPOITOWORLD_H
