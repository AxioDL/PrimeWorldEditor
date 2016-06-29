#include "CPoiToWorldLoader.h"

CPoiToWorld* CPoiToWorldLoader::LoadEGMC(IInputStream& rEGMC, CResourceEntry *pEntry)
{
    CPoiToWorld *pOut = new CPoiToWorld(pEntry);
    u32 NumMappings = rEGMC.ReadLong();

    for (u32 iMap = 0; iMap < NumMappings; iMap++)
    {
        u32 MeshID = rEGMC.ReadLong();
        u32 InstanceID = rEGMC.ReadLong();
        pOut->AddPoiMeshMap(InstanceID, MeshID);
    }

    return pOut;
}
