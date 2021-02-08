#include "CPoiToWorldCooker.h"

bool CPoiToWorldCooker::CookEGMC(CPoiToWorld *pPoiToWorld, IOutputStream& rOut)
{
    // Create mappings list
    struct SPoiMapping
    {
        uint32 MeshID;
        uint32 PoiID;
    };
    std::vector<SPoiMapping> Mappings;

    for (size_t iPoi = 0; iPoi < pPoiToWorld->NumMappedPOIs(); iPoi++)
    {
        const CPoiToWorld::SPoiMap *pkMap = pPoiToWorld->MapByIndex(iPoi);

        for (const auto meshID : pkMap->ModelIDs)
        {
            Mappings.push_back({meshID, pkMap->PoiID});
        }
    }

    // Write EGMC
    rOut.WriteULong(static_cast<uint32>(Mappings.size()));

    for (const auto& mapping : Mappings)
    {
        rOut.WriteULong(mapping.MeshID);
        rOut.WriteULong(mapping.PoiID);
    }

    return true;
}
