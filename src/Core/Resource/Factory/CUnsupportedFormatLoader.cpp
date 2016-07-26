#include "CUnsupportedFormatLoader.h"
#include "Core/Resource/ParticleParameters.h"

CDependencyGroup* CUnsupportedFormatLoader::LoadRULE(IInputStream& rRULE, CResourceEntry *pEntry)
{
    // RULE files can contain a reference to another RULE file, but has no other dependencies.
    u32 Magic = rRULE.ReadLong();
    ASSERT(CFourCC(Magic) == "RULE");

    CDependencyGroup *pOut = new CDependencyGroup(pEntry);
    rRULE.Seek(0x1, SEEK_CUR);

    // Version test
    u32 IDOffset = rRULE.Tell();
    rRULE.Seek(0x4, SEEK_CUR);
    u32 RuleSetCount = rRULE.ReadLong();
    EIDLength IDLength = (RuleSetCount > 0xFF ? e64Bit : e32Bit);
    rRULE.Seek(IDOffset, SEEK_SET);

    // Read rule ID
    CAssetID RuleID(rRULE, IDLength);
    pOut->AddDependency(RuleID);
    return pOut;
}
