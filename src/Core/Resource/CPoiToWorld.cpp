#include "CPoiToWorld.h"

CPoiToWorld::CPoiToWorld(CResourceEntry *pEntry)
    : CResource(pEntry)
{
}

CPoiToWorld::~CPoiToWorld() = default;

void CPoiToWorld::AddPoi(uint32 PoiID)
{
    // Check if this POI already exists
    const auto it = mPoiLookupMap.find(PoiID);

    if (it != mPoiLookupMap.end())
        return;

    auto pMap = std::make_unique<SPoiMap>();
    pMap->PoiID = PoiID;

    auto* ptr = mMaps.emplace_back(std::move(pMap)).get();
    mPoiLookupMap.insert_or_assign(PoiID, ptr);
}

void CPoiToWorld::AddPoiMeshMap(uint32 PoiID, uint32 ModelID)
{
    // Make sure the POI exists; the add function won't do anything if it does
    AddPoi(PoiID);
    SPoiMap *pMap = mPoiLookupMap[PoiID];

    // Check whether this model ID is already mapped to this POI
    for (const auto id : pMap->ModelIDs)
    {
        if (id == ModelID)
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
        return;

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
