#include "Core/Resource/Factory/CSkinLoader.h"

#include <Common/CFourCC.h>
#include <Common/FileIO/IInputStream.h>
#include "Core/Resource/Animation/CSkin.h"

// ************ STATIC ************
std::unique_ptr<CSkin> CSkinLoader::LoadCSKR(IInputStream& rCSKR, CResourceEntry *pEntry)
{
    if (!rCSKR.IsValid())
        return nullptr;

    auto pSkin = std::make_unique<CSkin>(pEntry);

    // We don't support MP3/DKCR CSKR yet
    if (rCSKR.PeekFourCC() == CFourCC("SKIN"))
        return pSkin;

    const auto NumVertexGroups = rCSKR.ReadU32();
    pSkin->mVertGroups.resize(NumVertexGroups);

    for (size_t iGrp = 0; iGrp < NumVertexGroups; iGrp++)
    {
        CSkin::SVertGroup& rGroup = pSkin->mVertGroups[iGrp];
        const auto NumWeights = rCSKR.ReadU32();
        ASSERT(NumWeights <= 4);

        for (size_t iWgt = 0; iWgt < NumWeights; iWgt++)
        {
            rGroup.Weights.Indices[iWgt] = static_cast<uint8_t>(rCSKR.ReadU32());
            rGroup.Weights.Weights[iWgt] = rCSKR.ReadF32();
        }

        rGroup.NumVertices = rCSKR.ReadU32();
    }

    return pSkin;
}
