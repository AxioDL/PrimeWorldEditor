#include "CSkinLoader.h"
#include <Common/AssertMacro.h>

// ************ STATIC ************
CSkin* CSkinLoader::LoadCSKR(IInputStream& rCSKR, CResourceEntry *pEntry)
{
    if (!rCSKR.IsValid()) return nullptr;
    CSkin *pSkin = new CSkin(pEntry);

    // We don't support MP3/DKCR CSKR yet
    if (rCSKR.PeekLong() == FOURCC('SKIN'))
        return pSkin;

    u32 NumVertexGroups = rCSKR.ReadLong();
    pSkin->mVertGroups.resize(NumVertexGroups);

    for (u32 iGrp = 0; iGrp < NumVertexGroups; iGrp++)
    {
        CSkin::SVertGroup& rGroup = pSkin->mVertGroups[iGrp];
        u32 NumWeights = rCSKR.ReadLong();
        ASSERT(NumWeights <= 4);

        for (u32 iWgt = 0; iWgt < NumWeights; iWgt++)
        {
            rGroup.Weights.Indices[iWgt] = (u8) rCSKR.ReadLong();
            rGroup.Weights.Weights[iWgt] = rCSKR.ReadFloat();
        }

        rGroup.NumVertices = rCSKR.ReadLong();
    }

    return pSkin;
}
