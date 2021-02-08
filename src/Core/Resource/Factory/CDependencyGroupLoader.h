#ifndef CDEPENDENCYGROUPLOADER_H
#define CDEPENDENCYGROUPLOADER_H

#include "Core/Resource/CDependencyGroup.h"
#include <Common/EGame.h>
#include <memory>

class CDependencyGroupLoader
{
    CDependencyGroupLoader() = default;
    static EGame VersionTest(IInputStream& rDGRP, uint32 DepCount);

public:
    static std::unique_ptr<CDependencyGroup> LoadDGRP(IInputStream& rDGRP, CResourceEntry *pEntry);
};

#endif // CDEPENDENCYGROUPLOADER_H
