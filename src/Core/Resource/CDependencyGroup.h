#ifndef CDEPENDENCYGROUP
#define CDEPENDENCYGROUP

#include "CResource.h"

class CDependencyGroup : public CResource
{
    DECLARE_RESOURCE_TYPE(DependencyGroup)
    std::vector<CAssetID> mDependencies;

public:
    explicit CDependencyGroup(CResourceEntry *pEntry = nullptr) : CResource(pEntry) {}

    void Clear()                                     { mDependencies.clear(); }
    uint32 NumDependencies() const                   { return mDependencies.size(); }
    CAssetID DependencyByIndex(uint32 Index) const   { return mDependencies[Index]; }

    void AddDependency(const CAssetID& rkID)
    {
        if (!HasDependency(rkID))
            mDependencies.push_back(rkID);
    }

    void AddDependency(CResource *pRes)
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

    std::unique_ptr<CDependencyTree> BuildDependencyTree() const
    {
        auto pTree = std::make_unique<CDependencyTree>();

        for (const auto& dep : mDependencies)
            pTree->AddDependency(dep);

        return pTree;
    }
};

#endif // CDEPENDENCYGROUP

