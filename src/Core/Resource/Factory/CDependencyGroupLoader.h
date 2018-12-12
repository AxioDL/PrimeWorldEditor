#ifndef CDEPENDENCYGROUPLOADER_H
#define CDEPENDENCYGROUPLOADER_H

#include "Core/Resource/CDependencyGroup.h"
#include <Common/EGame.h>

class CDependencyGroupLoader
{
    CDependencyGroupLoader() {}
    static EGame VersionTest(IInputStream& rDGRP, uint32 DepCount);

public:
    static CDependencyGroup* LoadDGRP(IInputStream& rDGRP, CResourceEntry *pEntry);
};

#endif // CDEPENDENCYGROUPLOADER_H
