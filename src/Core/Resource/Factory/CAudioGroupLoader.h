#ifndef CAUDIOGROUPLOADER_H
#define CAUDIOGROUPLOADER_H

#include "Core/Resource/CAudioGroup.h"
#include "Core/Resource/CAudioLookupTable.h"
#include "Core/Resource/CStringList.h"

class CAudioGroupLoader
{
    CAudioGroupLoader() {}

public:
    static CAudioGroup* LoadAGSC(IInputStream& rAGSC, CResourceEntry *pEntry);
    static CAudioLookupTable* LoadATBL(IInputStream& rATBL, CResourceEntry *pEntry);
    static CStringList* LoadSTLC(IInputStream& rSTLC, CResourceEntry *pEntry);
};

#endif // CAUDIOGROUPLOADER_H
