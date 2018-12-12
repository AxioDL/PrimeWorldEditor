#include "CDependencyGroupLoader.h"
#include <Common/Macros.h>

EGame CDependencyGroupLoader::VersionTest(IInputStream& rDGRP, uint32 DepCount)
{
    // Only difference between versions is asset ID length. Just check for EOF with 32-bit ID length.
    uint32 Start = rDGRP.Tell();
    rDGRP.Seek(DepCount * 8, SEEK_CUR);
    uint32 Remaining = rDGRP.Size() - rDGRP.Tell();

    EGame Game = EGame::CorruptionProto;

    if (Remaining < 32)
    {
        bool IsEOF = true;

        for (uint32 iRem = 0; iRem < Remaining; iRem++)
        {
            uint8 Byte = rDGRP.ReadByte();

            if (Byte != 0xFF)
            {
                IsEOF = false;
                break;
            }
        }

        if (IsEOF) Game = EGame::PrimeDemo;
    }

    rDGRP.Seek(Start, SEEK_SET);
    return Game;
}

CDependencyGroup* CDependencyGroupLoader::LoadDGRP(IInputStream& rDGRP, CResourceEntry *pEntry)
{
    if (!rDGRP.IsValid()) return nullptr;

    uint32 NumDependencies = rDGRP.ReadLong();
    EGame Game = VersionTest(rDGRP, NumDependencies);

    CDependencyGroup *pGroup = new CDependencyGroup(pEntry);

    for (uint32 iDep = 0; iDep < NumDependencies; iDep++)
    {
        rDGRP.Seek(0x4, SEEK_CUR); // Skip dependency type
        CAssetID AssetID(rDGRP, Game);
        pGroup->AddDependency(AssetID);
    }

    return pGroup;
}
