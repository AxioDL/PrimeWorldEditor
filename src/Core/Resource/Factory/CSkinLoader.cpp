#include "CSkinLoader.h"
#include <Common/Macros.h>

// ************ STATIC ************
std::unique_ptr<CSkin> CSkinLoader::LoadCSKR(IInputStream& rCSKR, CResourceEntry *pEntry)
{
    if (!rCSKR.IsValid())
        return nullptr;

    auto pSkin = std::make_unique<CSkin>(pEntry);

    // We don't support MP3/DKCR CSKR yet
    if (rCSKR.PeekLong() == FOURCC('SKIN'))
        return pSkin;

    const uint32 NumVertexGroups = rCSKR.ReadULong();
    pSkin->mVertGroups.resize(NumVertexGroups);

    for (size_t iGrp = 0; iGrp < NumVertexGroups; iGrp++)
    {
        CSkin::SVertGroup& rGroup = pSkin->mVertGroups[iGrp];
        const uint32 NumWeights = rCSKR.ReadULong();
        ASSERT(NumWeights <= 4);

        for (size_t iWgt = 0; iWgt < NumWeights; iWgt++)
        {
            rGroup.Weights.Indices[iWgt] = static_cast<uint8>(rCSKR.ReadULong());
            rGroup.Weights.Weights[iWgt] = rCSKR.ReadFloat();
        }

        rGroup.NumVertices = rCSKR.ReadULong();
    }

    return pSkin;
}
