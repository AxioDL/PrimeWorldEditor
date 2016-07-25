#ifndef CDEPENDENCYGROUP
#define CDEPENDENCYGROUP

#include "CResource.h"

class CDependencyGroup : public CResource
{
    DECLARE_RESOURCE_TYPE(eDependencyGroup)
    std::set<CUniqueID> mDependencies;

public:
    CDependencyGroup(CResourceEntry *pEntry = 0) : CResource(pEntry) {}
    inline void AddDependency(const CUniqueID& rkID)        { mDependencies.insert(rkID); }
    inline void AddDependency(CResource *pRes)              { if (pRes) mDependencies.insert(pRes->ResID()); }
    inline void RemoveDependency(const CUniqueID& rkID)     { mDependencies.erase(rkID); }
    inline void Clear()                                     { mDependencies.clear(); }
    inline bool HasDependency(const CUniqueID& rkID) const  { return mDependencies.find(rkID) != mDependencies.end(); }
    inline u32 NumDependencies() const                      { return mDependencies.size(); }
    inline CUniqueID DependencyByIndex(u32 Index) const     { return *std::next(mDependencies.begin(), Index); }
    
    CDependencyTree* BuildDependencyTree() const
    {
        CDependencyTree *pTree = new CDependencyTree(ResID());

        for (auto DepIt = mDependencies.begin(); DepIt != mDependencies.end(); DepIt++)
            pTree->AddDependency(*DepIt);

        return pTree;
    }
};

#endif // CDEPENDENCYGROUP

