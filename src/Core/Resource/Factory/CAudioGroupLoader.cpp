#include "CAudioGroupLoader.h"

CAudioGroup* CAudioGroupLoader::LoadAGSC(IInputStream& rAGSC, CResourceEntry *pEntry)
{
    // For now we only load sound define IDs and the group ID!
    // Version check
    uint32 Check = rAGSC.PeekLong();
    EGame Game = (Check == 0x1 ? EGame::Echoes : EGame::Prime);
    CAudioGroup *pOut = new CAudioGroup(pEntry);

    // Read header, navigate to Proj chunk
    if (Game == EGame::Prime)
    {
        rAGSC.ReadString();
        pOut->mGroupName = rAGSC.ReadString();
        uint32 PoolSize = rAGSC.ReadLong();
        rAGSC.Seek(PoolSize + 0x4, SEEK_CUR);
    }

    else
    {
        rAGSC.Seek(0x4, SEEK_CUR);
        pOut->mGroupName = rAGSC.ReadString();
        pOut->mGroupID = rAGSC.ReadShort();
        uint32 PoolSize = rAGSC.ReadLong();
        rAGSC.Seek(0xC + PoolSize, SEEK_CUR);
    }

    // Read needed data from the Proj chunk
    uint16 Peek = rAGSC.PeekShort();

    if (Peek != 0xFFFF)
    {
        uint32 ProjStart = rAGSC.Tell();
        rAGSC.Seek(0x4, SEEK_CUR);
        uint16 GroupID = rAGSC.ReadShort();
        uint16 GroupType = rAGSC.ReadShort();
        rAGSC.Seek(0x14, SEEK_CUR);
        uint32 SfxTableStart = rAGSC.ReadLong();

        if (Game == EGame::Prime)
            pOut->mGroupID = GroupID;
        else
            ASSERT(pOut->mGroupID == GroupID);

        if (GroupType == 1)
        {
            rAGSC.Seek(ProjStart + SfxTableStart, SEEK_SET);
            uint16 NumSounds = rAGSC.ReadShort();
            rAGSC.Seek(0x2, SEEK_CUR);

            for (uint32 iSfx = 0; iSfx < NumSounds; iSfx++)
            {
                pOut->mDefineIDs.push_back( rAGSC.ReadShort() );
                rAGSC.Seek(0x8, SEEK_CUR);
            }
        }
    }

    return pOut;
}

CAudioLookupTable* CAudioGroupLoader::LoadATBL(IInputStream& rATBL, CResourceEntry *pEntry)
{
    CAudioLookupTable *pOut = new CAudioLookupTable(pEntry);
    uint32 NumMacroIDs = rATBL.ReadLong();

    for (uint32 iMacro = 0; iMacro < NumMacroIDs; iMacro++)
        pOut->mDefineIDs.push_back( rATBL.ReadShort() );

    return pOut;
}

CStringList* CAudioGroupLoader::LoadSTLC(IInputStream& rSTLC, CResourceEntry *pEntry)
{
    CStringList *pOut = new CStringList(pEntry);
    uint32 NumStrings = rSTLC.ReadLong();
    pOut->mStringList.reserve(NumStrings);

    for (uint32 iStr = 0; iStr < NumStrings; iStr++)
        pOut->mStringList.push_back( rSTLC.ReadString() );

    return pOut;
}
