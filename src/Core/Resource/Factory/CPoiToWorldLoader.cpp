#include "CPoiToWorldLoader.h"

std::unique_ptr<CPoiToWorld> CPoiToWorldLoader::LoadEGMC(IInputStream& rEGMC, CResourceEntry *pEntry)
{
    auto pOut = std::make_unique<CPoiToWorld>(pEntry);
    uint32 NumMappings = rEGMC.ReadLong();

    for (uint32 iMap = 0; iMap < NumMappings; iMap++)
    {
        uint32 MeshID = rEGMC.ReadLong();
        uint32 InstanceID = rEGMC.ReadLong() & 0x03FFFFFF;
        pOut->AddPoiMeshMap(InstanceID, MeshID);
    }

    return pOut;
}
