#ifndef CAUDIOGROUPLOADER_H
#define CAUDIOGROUPLOADER_H

#include "Core/Resource/CAudioGroup.h"
#include "Core/Resource/CAudioLookupTable.h"
#include "Core/Resource/CStringList.h"
#include <memory>

class CAudioGroupLoader
{
    CAudioGroupLoader() = default;

public:
    static std::unique_ptr<CAudioGroup> LoadAGSC(IInputStream& rAGSC, CResourceEntry *pEntry);
    static std::unique_ptr<CAudioLookupTable> LoadATBL(IInputStream& rATBL, CResourceEntry *pEntry);
    static std::unique_ptr<CStringList> LoadSTLC(IInputStream& rSTLC, CResourceEntry *pEntry);
};

#endif // CAUDIOGROUPLOADER_H
