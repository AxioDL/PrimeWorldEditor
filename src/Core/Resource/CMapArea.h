#ifndef CMAPAREA_H
#define CMAPAREA_H

#include "CResource.h"

// Barebones class
class CMapArea : public CResource
{
    friend class CUnsupportedFormatLoader;
    CAssetID mNameString;

public:
    CMapArea(CResourceEntry *pEntry = 0)
        : CResource(pEntry)
    {}

    CDependencyTree* BuildDependencyTree() const
    {
        CDependencyTree *pTree = new CDependencyTree();
        pTree->AddDependency(mNameString);
        return pTree;
    }
};

#endif // CMAPAREA_H
