#ifndef CDEPENDENCYGROUP
#define CDEPENDENCYGROUP

#include "CResource.h"
#include <algorithm>

class CDependencyGroup : public CResource
{
    DECLARE_RESOURCE_TYPE(DependencyGroup)
    std::vector<CAssetID> mDependencies;

public:
    explicit CDependencyGroup(CResourceEntry *pEntry = nullptr) : CResource(pEntry) {}

    void Clear()                                     { mDependencies.clear(); }
    uint32 NumDependencies() const                   { return mDependencies.size(); }
    CAssetID DependencyByIndex(size_t Index) const   { return mDependencies[Index]; }

    void AddDependency(const CAssetID& rkID)
    {
        if (!HasDependency(rkID))
            mDependencies.push_back(rkID);
    }

    void AddDependency(const CResource* pRes)
    {
        if (pRes != nullptr && !HasDependency(pRes->ID()))
            mDependencies.push_back(pRes->ID());
    }

    void RemoveDependency(const CAssetID& rkID)
    {
        const auto it = std::find_if(mDependencies.cbegin(), mDependencies.cend(),
                                     [&rkID](const auto& entry) { return entry == rkID; });

        if (it == mDependencies.cend())
            return;

        mDependencies.erase(it);
    }
    
    bool HasDependency(const CAssetID& rkID) const
    {
        return std::any_of(mDependencies.cbegin(), mDependencies.cend(),
                           [&rkID](const auto& entry) { return entry == rkID; });
    }

    std::unique_ptr<CDependencyTree> BuildDependencyTree() const override
    {
        auto pTree = std::make_unique<CDependencyTree>();

        for (const auto& dep : mDependencies)
            pTree->AddDependency(dep);

        return pTree;
    }
};

#endif // CDEPENDENCYGROUP

