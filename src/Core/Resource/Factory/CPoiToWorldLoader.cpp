#include "CPoiToWorldLoader.h"

std::unique_ptr<CPoiToWorld> CPoiToWorldLoader::LoadEGMC(IInputStream& rEGMC, CResourceEntry *pEntry)
{
    auto pOut = std::make_unique<CPoiToWorld>(pEntry);
    const uint32 NumMappings = rEGMC.ReadULong();

    for (uint32 iMap = 0; iMap < NumMappings; iMap++)
    {
        const uint32 MeshID = rEGMC.ReadULong();
        const uint32 InstanceID = rEGMC.ReadULong() & 0x03FFFFFF;
        pOut->AddPoiMeshMap(InstanceID, MeshID);
    }

    return pOut;
}
