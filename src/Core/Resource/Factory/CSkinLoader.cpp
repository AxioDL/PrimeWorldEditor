#include "CSkinLoader.h"
#include <Common/AssertMacro.h>

// ************ STATIC ************
CSkin* CSkinLoader::LoadCSKR(IInputStream& rCSKR, CResourceEntry *pEntry)
{
    if (!rCSKR.IsValid()) return nullptr;

    u32 NumVertexGroups = rCSKR.ReadLong();
    CSkin *pSkin = new CSkin(pEntry);
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
