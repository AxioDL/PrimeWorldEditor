#include "CPoiToWorldLoader.h"

CPoiToWorld* CPoiToWorldLoader::LoadEGMC(IInputStream& rEGMC, CResourceEntry *pEntry)
{
    CPoiToWorld *pOut = new CPoiToWorld(pEntry);
    uint32 NumMappings = rEGMC.ReadLong();

    for (uint32 iMap = 0; iMap < NumMappings; iMap++)
    {
        uint32 MeshID = rEGMC.ReadLong();
        uint32 InstanceID = rEGMC.ReadLong() & 0x03FFFFFF;
        pOut->AddPoiMeshMap(InstanceID, MeshID);
    }

    return pOut;
}
