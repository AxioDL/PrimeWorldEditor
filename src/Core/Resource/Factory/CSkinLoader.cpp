#include "CSkinLoader.h"
#include <Common/Macros.h>

// ************ STATIC ************
CSkin* CSkinLoader::LoadCSKR(IInputStream& rCSKR, CResourceEntry *pEntry)
{
    if (!rCSKR.IsValid()) return nullptr;
    CSkin *pSkin = new CSkin(pEntry);

    // We don't support MP3/DKCR CSKR yet
    if (rCSKR.PeekLong() == FOURCC('SKIN'))
        return pSkin;

    uint32 NumVertexGroups = rCSKR.ReadLong();
    pSkin->mVertGroups.resize(NumVertexGroups);

    for (uint32 iGrp = 0; iGrp < NumVertexGroups; iGrp++)
    {
        CSkin::SVertGroup& rGroup = pSkin->mVertGroups[iGrp];
        uint32 NumWeights = rCSKR.ReadLong();
        ASSERT(NumWeights <= 4);

        for (uint32 iWgt = 0; iWgt < NumWeights; iWgt++)
        {
            rGroup.Weights.Indices[iWgt] = (uint8) rCSKR.ReadLong();
            rGroup.Weights.Weights[iWgt] = rCSKR.ReadFloat();
        }

        rGroup.NumVertices = rCSKR.ReadLong();
    }

    return pSkin;
}
