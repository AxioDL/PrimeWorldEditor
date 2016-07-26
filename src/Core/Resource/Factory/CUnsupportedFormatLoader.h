#ifndef CUNSUPPORTEDFORMATLOADER_H
#define CUNSUPPORTEDFORMATLOADER_H

#include "Core/Resource/CDependencyGroup.h"

// This class is responsible for loading formats that aren't yet fully supported.
// This is needed so we have access to the full dependency list of all resource types.
class CUnsupportedFormatLoader
{
    CDependencyGroup *mpGroup;
    CUnsupportedFormatLoader() {}

public:
    static CDependencyGroup* LoadRULE(IInputStream& rRULE, CResourceEntry *pEntry);
    static CDependencyGroup* LoadPART(IInputStream& rPART, CResourceEntry *pEntry);
};

#endif // CUNSUPPORTEDFORMATLOADER_H
