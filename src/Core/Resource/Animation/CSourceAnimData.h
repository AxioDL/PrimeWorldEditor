#ifndef CSOURCEANIMDATA_H
#define CSOURCEANIMDATA_H

#include "Core/Resource/CResource.h"
#include "IMetaTransition.h"

#include <memory>

class CSourceAnimData : public CResource
{
    DECLARE_RESOURCE_TYPE(SourceAnimData)
    friend class CAnimSetLoader;

    struct STransition
    {
        CAssetID AnimA;
        CAssetID AnimB;
        std::unique_ptr<IMetaTransition> pTransition;
    };

    struct SHalfTransition
    {
        CAssetID Anim;
        std::unique_ptr<IMetaTransition> pTransition;
    };

    std::vector<STransition> mTransitions;
    std::vector<SHalfTransition> mHalfTransitions;
    std::unique_ptr<IMetaTransition> mpDefaultTransition;

public:
    explicit CSourceAnimData(CResourceEntry *pEntry = nullptr)
        : CResource(pEntry)
    {}

    ~CSourceAnimData() override = default;

    std::unique_ptr<CDependencyTree> BuildDependencyTree() const override
    {
        // SAND normally has dependencies from meta-transitions and events
        // However, all of these can be character-specific. To simplify things, all SAND
        // dependencies are being added to the CHAR dependency tree instead. Therefore the
        // SAND dependency tree is left empty.
        return std::make_unique<CDependencyTree>();
    }

    void GetUniquePrimitives(std::set<CAnimPrimitive>& rPrimSet) const
    {
        for (const auto& transition : mTransitions)
            transition.pTransition->GetUniquePrimitives(rPrimSet);

        for (const auto& halfTrans : mHalfTransitions)
            halfTrans.pTransition->GetUniquePrimitives(rPrimSet);

        if (mpDefaultTransition)
            mpDefaultTransition->GetUniquePrimitives(rPrimSet);
    }

    void AddTransitionDependencies(CDependencyTree *pTree)
    {
        // Note: All CHAR animations must have been added to the tree before this function is run
        std::set<IMetaTransition*> UsedTransitions;

        while (true)
        {
            // Find all relevant primitives
            std::set<CAnimPrimitive> PrimSet;

            if (UsedTransitions.find(mpDefaultTransition.get()) == UsedTransitions.cend())
            {
                mpDefaultTransition->GetUniquePrimitives(PrimSet);
                UsedTransitions.insert(mpDefaultTransition.get());
            }

            for (const STransition& transition : mTransitions)
            {
                IMetaTransition *pTransition = transition.pTransition.get();

                if (pTree->HasDependency(transition.AnimA) &&
                    pTree->HasDependency(transition.AnimB) &&
                    UsedTransitions.find(pTransition) == UsedTransitions.cend())
                {
                    pTransition->GetUniquePrimitives(PrimSet);
                    UsedTransitions.insert(pTransition);
                }
            }

            for (const SHalfTransition& halfTrans : mHalfTransitions)
            {
                IMetaTransition *pTransition = halfTrans.pTransition.get();

                if (pTree->HasDependency(halfTrans.Anim) &&
                    UsedTransitions.find(pTransition) == UsedTransitions.cend())
                {
                    pTransition->GetUniquePrimitives(PrimSet);
                    UsedTransitions.insert(pTransition);
                }
            }

            // If we have no new primitives then we've exhausted all usable transitions; break out
            if (PrimSet.empty())
                break;

            // Add all transition primitives to the tree
            for (const auto& primitive : PrimSet)
                pTree->AddDependency(primitive.Animation());
        }
    }
};

#endif // CSOURCEANIMDATA_H
