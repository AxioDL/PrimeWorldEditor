#include "CPoiToWorld.h"

#include <algorithm>

CPoiToWorld::CPoiToWorld(CResourceEntry *pEntry /*= 0*/)
    : CResource(pEntry)
{
}

CPoiToWorld::~CPoiToWorld() = default;

void CPoiToWorld::AddPoi(uint32 PoiID)
{
    // Check if this POI already exists
    const auto it = mPoiLookupMap.find(PoiID);
    if (it != mPoiLookupMap.cend())
    {
        return;
    }

    auto pMap = std::make_unique<SPoiMap>();
    pMap->PoiID = PoiID;

    mPoiLookupMap.insert_or_assign(PoiID, pMap.get());
    mMaps.push_back(std::move(pMap));
}

void CPoiToWorld::AddPoiMeshMap(uint32 PoiID, uint32 ModelID)
{
    // Make sure the POI exists; the add function won't do anything if it does
    AddPoi(PoiID);
    SPoiMap *pMap = mPoiLookupMap[PoiID];

    // Check whether this model ID is already mapped to this POI
    const bool exists = std::any_of(pMap->ModelIDs.cbegin(), pMap->ModelIDs.cend(),
                                    [ModelID](const auto ID) { return ID == ModelID; });
    if (exists)
    {
        return;
    }

    // We didn't return, so this is a new mapping
    pMap->ModelIDs.push_back(ModelID);
}

void CPoiToWorld::RemovePoi(uint32 PoiID)
{
    for (auto it = mMaps.begin(); it != mMaps.end(); ++it)
    {
        if ((*it)->PoiID == PoiID)
        {
            mMaps.erase(it);
            mPoiLookupMap.erase(PoiID);
            return;
        }
    }
}

void CPoiToWorld::RemovePoiMeshMap(uint32 PoiID, uint32 ModelID)
{
    const auto MapIt = mPoiLookupMap.find(PoiID);
    if (MapIt == mPoiLookupMap.end())
    {
        return;
    }

    SPoiMap *pMap = MapIt->second;
    for (auto ListIt = pMap->ModelIDs.begin(); ListIt != pMap->ModelIDs.end(); ++ListIt)
    {
        if (*ListIt == ModelID)
        {
            pMap->ModelIDs.erase(ListIt);
            break;
        }
    }
}
