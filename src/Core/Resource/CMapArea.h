#ifndef CMAPAREA_H
#define CMAPAREA_H

#include "CResource.h"

// Barebones class
class CMapArea : public CResource
{
    friend class CUnsupportedFormatLoader;
    CAssetID mNameString;

public:
    CMapArea(CResourceEntry *pEntry = nullptr)
        : CResource(pEntry)
    {}

    CDependencyTree* BuildDependencyTree() const override
    {
        auto *pTree = new CDependencyTree();
        pTree->AddDependency(mNameString);
        return pTree;
    }
};

#endif // CMAPAREA_H
