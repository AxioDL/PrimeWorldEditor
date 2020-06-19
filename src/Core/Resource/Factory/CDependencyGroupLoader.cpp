#include "CDependencyGroupLoader.h"
#include <Common/Macros.h>

EGame CDependencyGroupLoader::VersionTest(IInputStream& rDGRP, uint32 DepCount)
{
    // Only difference between versions is asset ID length. Just check for EOF with 32-bit ID length.
    const uint32 Start = rDGRP.Tell();
    rDGRP.Seek(DepCount * 8, SEEK_CUR);
    const uint32 Remaining = rDGRP.Size() - rDGRP.Tell();

    EGame Game = EGame::CorruptionProto;

    if (Remaining < 32)
    {
        bool IsEOF = true;

        for (uint32 iRem = 0; iRem < Remaining; iRem++)
        {
            const uint8 Byte = rDGRP.ReadUByte();

            if (Byte != 0xFF)
            {
                IsEOF = false;
                break;
            }
        }

        if (IsEOF)
            Game = EGame::PrimeDemo;
    }

    rDGRP.Seek(Start, SEEK_SET);
    return Game;
}

std::unique_ptr<CDependencyGroup> CDependencyGroupLoader::LoadDGRP(IInputStream& rDGRP, CResourceEntry *pEntry)
{
    if (!rDGRP.IsValid())
        return nullptr;

    const uint32 NumDependencies = rDGRP.ReadULong();
    const EGame Game = VersionTest(rDGRP, NumDependencies);

    auto pGroup = std::make_unique<CDependencyGroup>(pEntry);

    for (uint32 iDep = 0; iDep < NumDependencies; iDep++)
    {
        rDGRP.Seek(0x4, SEEK_CUR); // Skip dependency type
        const CAssetID AssetID(rDGRP, Game);
        pGroup->AddDependency(AssetID);
    }

    return pGroup;
}
