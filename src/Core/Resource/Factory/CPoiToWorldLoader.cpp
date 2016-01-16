#include "CPoiToWorldLoader.h"

CPoiToWorld* CPoiToWorldLoader::LoadEGMC(IInputStream& rEGMC)
{
    CPoiToWorld *pOut = new CPoiToWorld();
    u32 NumMappings = rEGMC.ReadLong();

    for (u32 iMap = 0; iMap < NumMappings; iMap++)
    {
        u32 MeshID = rEGMC.ReadLong();
        u32 InstanceID = rEGMC.ReadLong();
        pOut->AddPoiMeshMap(InstanceID, MeshID);
    }

    return pOut;
}
