#ifndef CDEPENDENCYGROUP
#define CDEPENDENCYGROUP

#include "CResource.h"

class CDependencyGroup : public CResource
{
    DECLARE_RESOURCE_TYPE(eDependencyGroup)
    std::vector<CAssetID> mDependencies;

public:
    CDependencyGroup(CResourceEntry *pEntry = 0) : CResource(pEntry) {}

    inline void Clear()                                     { mDependencies.clear(); }
    inline uint32 NumDependencies() const                   { return mDependencies.size(); }
    inline CAssetID DependencyByIndex(uint32 Index) const   { return mDependencies[Index]; }

    inline void AddDependency(const CAssetID& rkID)
    {
        if (!HasDependency(rkID))
            mDependencies.push_back(rkID);
    }

    inline void AddDependency(CResource *pRes)
    {
        if ( pRes && !HasDependency(pRes->ID()) )
            mDependencies.push_back(pRes->ID());
    }

    void RemoveDependency(const CAssetID& rkID)
    {
        for (auto Iter = mDependencies.begin(); Iter != mDependencies.end(); Iter++)
        {
            if (*Iter == rkID)
            {
                mDependencies.erase(Iter);
                return;
            }
        }
    }
    
    bool HasDependency(const CAssetID &rkID) const
    {
        for (uint32 iDep = 0; iDep < mDependencies.size(); iDep++)
        {
            if (mDependencies[iDep] == rkID)
                return true;
        }

        return false;
    }

    CDependencyTree* BuildDependencyTree() const
    {
        CDependencyTree *pTree = new CDependencyTree();

        for (auto DepIt = mDependencies.begin(); DepIt != mDependencies.end(); DepIt++)
            pTree->AddDependency(*DepIt);

        return pTree;
    }
};

#endif // CDEPENDENCYGROUP

