#include "CAudioGroupLoader.h"

std::unique_ptr<CAudioGroup> CAudioGroupLoader::LoadAGSC(IInputStream& rAGSC, CResourceEntry *pEntry)
{
    // For now we only load sound define IDs and the group ID!
    // Version check
    const uint32 Check = rAGSC.PeekULong();
    const EGame Game = Check == 0x1 ? EGame::Echoes : EGame::Prime;
    auto pOut = std::make_unique<CAudioGroup>(pEntry);

    // Read header, navigate to Proj chunk
    if (Game == EGame::Prime)
    {
        rAGSC.ReadString();
        pOut->mGroupName = rAGSC.ReadString();
        const uint32 PoolSize = rAGSC.ReadULong();
        rAGSC.Seek(PoolSize + 0x4, SEEK_CUR);
    }

    else
    {
        rAGSC.Seek(0x4, SEEK_CUR);
        pOut->mGroupName = rAGSC.ReadString();
        pOut->mGroupID = rAGSC.ReadUShort();
        const uint32 PoolSize = rAGSC.ReadULong();
        rAGSC.Seek(0xC + PoolSize, SEEK_CUR);
    }

    // Read needed data from the Proj chunk
    const uint16 Peek = rAGSC.PeekUShort();

    if (Peek != 0xFFFF)
    {
        const uint32 ProjStart = rAGSC.Tell();
        rAGSC.Seek(0x4, SEEK_CUR);
        const uint16 GroupID = rAGSC.ReadUShort();
        const uint16 GroupType = rAGSC.ReadUShort();
        rAGSC.Seek(0x14, SEEK_CUR);
        const uint32 SfxTableStart = rAGSC.ReadULong();

        if (Game == EGame::Prime)
            pOut->mGroupID = GroupID;
        else
            ASSERT(pOut->mGroupID == GroupID);

        if (GroupType == 1)
        {
            rAGSC.Seek(ProjStart + SfxTableStart, SEEK_SET);
            const uint16 NumSounds = rAGSC.ReadUShort();
            rAGSC.Seek(0x2, SEEK_CUR);

            for (uint32 iSfx = 0; iSfx < NumSounds; iSfx++)
            {
                pOut->mDefineIDs.push_back(rAGSC.ReadUShort());
                rAGSC.Seek(0x8, SEEK_CUR);
            }
        }
    }

    return pOut;
}

std::unique_ptr<CAudioLookupTable> CAudioGroupLoader::LoadATBL(IInputStream& rATBL, CResourceEntry *pEntry)
{
    auto pOut = std::make_unique<CAudioLookupTable>(pEntry);
    const uint32 NumMacroIDs = rATBL.ReadULong();

    for (uint32 iMacro = 0; iMacro < NumMacroIDs; iMacro++)
        pOut->mDefineIDs.push_back( rATBL.ReadShort() );

    return pOut;
}

std::unique_ptr<CStringList> CAudioGroupLoader::LoadSTLC(IInputStream& rSTLC, CResourceEntry *pEntry)
{
    auto pOut = std::make_unique<CStringList>(pEntry);
    const uint32 NumStrings = rSTLC.ReadULong();
    pOut->mStringList.reserve(NumStrings);

    for (uint32 iStr = 0; iStr < NumStrings; iStr++)
        pOut->mStringList.push_back(rSTLC.ReadString());

    return pOut;
}
