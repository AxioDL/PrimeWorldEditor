#include "CPoiToWorld.h"

CPoiToWorld::CPoiToWorld()
{
}

CPoiToWorld::~CPoiToWorld()
{
    for (auto it = mMaps.begin(); it != mMaps.end(); it++)
        delete *it;
}

void CPoiToWorld::AddPoi(u32 PoiID)
{
    // Check if this POI already exists
    auto it = mPoiLookupMap.find(PoiID);

    if (it == mPoiLookupMap.end())
    {
        SPoiMap *pMap = new SPoiMap();
        pMap->PoiID = PoiID;

        mMaps.push_back(pMap);
        mPoiLookupMap[PoiID] = pMap;
    }
}

void CPoiToWorld::AddPoiMeshMap(u32 PoiID, u32 ModelID)
{
    // Make sure the POI exists; the add function won't do anything if it does
    AddPoi(PoiID);
    SPoiMap *pMap = mPoiLookupMap[PoiID];

    // Check whether this model ID is already mapped to this POI
    for (auto it = pMap->ModelIDs.begin(); it != pMap->ModelIDs.end(); it++)
    {
        if (*it == ModelID)
            return;
    }

    // We didn't return, so this is a new mapping
    pMap->ModelIDs.push_back(ModelID);
}

void CPoiToWorld::RemovePoi(u32 PoiID)
{
    for (auto it = mMaps.begin(); it != mMaps.end(); it++)
    {
        if ((*it)->PoiID == PoiID)
        {
            mMaps.erase(it);
            mPoiLookupMap.erase(PoiID);
            return;
        }
    }
}

void CPoiToWorld::RemovePoiMeshMap(u32 PoiID, u32 ModelID)
{
    auto MapIt = mPoiLookupMap.find(PoiID);

    if (MapIt != mPoiLookupMap.end())
    {
        SPoiMap *pMap = MapIt->second;

        for (auto ListIt = pMap->ModelIDs.begin(); ListIt != pMap->ModelIDs.end(); ListIt++)
        {
            if (*ListIt == ModelID)
            {
                pMap->ModelIDs.erase(ListIt);

                if (pMap->ModelIDs.empty())
                    RemovePoi(PoiID);

                break;
            }
        }
    }
}
