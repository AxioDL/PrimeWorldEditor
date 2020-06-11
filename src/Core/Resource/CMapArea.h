#ifndef CMAPAREA_H
#define CMAPAREA_H

#include "CResource.h"

// Barebones class
class CMapArea : public CResource
{
    friend class CUnsupportedFormatLoader;
    CAssetID mNameString;

public:
    explicit CMapArea(CResourceEntry *pEntry = nullptr)
        : CResource(pEntry)
    {}

    std::unique_ptr<CDependencyTree> BuildDependencyTree() const override
    {
        auto pTree = std::make_unique<CDependencyTree>();
        pTree->AddDependency(mNameString);
        return pTree;
    }
};

#endif // CMAPAREA_H
