#include "CPoiToWorldLoader.h"

CPoiToWorld* CPoiToWorldLoader::LoadEGMC(IInputStream& rEGMC)
{
    CPoiToWorld *pOut = new CPoiToWorld();
    u32 NumLinks = rEGMC.ReadLong();

    for (u32 iLink = 0; iLink < NumLinks; iLink++)
    {
        CPoiToWorld::SPoiMeshLink Link;
        Link.MeshID = rEGMC.ReadLong();
        Link.PoiInstanceID = rEGMC.ReadLong();
        pOut->mMeshLinks.push_back(Link);
    }

    return pOut;
}
