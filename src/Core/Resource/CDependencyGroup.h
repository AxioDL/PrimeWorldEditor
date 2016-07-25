#ifndef CDEPENDENCYGROUP
#define CDEPENDENCYGROUP

#include "CResource.h"

class CDependencyGroup : public CResource
{
    DECLARE_RESOURCE_TYPE(eDependencyGroup)
    std::set<CAssetID> mDependencies;

public:
    CDependencyGroup(CResourceEntry *pEntry = 0) : CResource(pEntry) {}
    inline void AddDependency(const CAssetID& rkID)         { mDependencies.insert(rkID); }
    inline void AddDependency(CResource *pRes)              { if (pRes) mDependencies.insert(pRes->ID()); }
    inline void RemoveDependency(const CAssetID& rkID)      { mDependencies.erase(rkID); }
    inline void Clear()                                     { mDependencies.clear(); }
    inline bool HasDependency(const CAssetID& rkID) const   { return mDependencies.find(rkID) != mDependencies.end(); }
    inline u32 NumDependencies() const                      { return mDependencies.size(); }
    inline CAssetID DependencyByIndex(u32 Index) const      { return *std::next(mDependencies.begin(), Index); }
    
    CDependencyTree* BuildDependencyTree() const
    {
        CDependencyTree *pTree = new CDependencyTree(ID());

        for (auto DepIt = mDependencies.begin(); DepIt != mDependencies.end(); DepIt++)
            pTree->AddDependency(*DepIt);

        return pTree;
    }
};

#endif // CDEPENDENCYGROUP

