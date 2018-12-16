#ifndef CSOURCEANIMDATA_H
#define CSOURCEANIMDATA_H

#include "Core/Resource/CResource.h"
#include "IMetaTransition.h"

class CSourceAnimData : public CResource
{
    DECLARE_RESOURCE_TYPE(SourceAnimData)
    friend class CAnimSetLoader;

    struct STransition
    {
        CAssetID AnimA;
        CAssetID AnimB;
        IMetaTransition *pTransition;
    };

    struct SHalfTransition
    {
        CAssetID Anim;
        IMetaTransition *pTransition;
    };

    std::vector<STransition> mTransitions;
    std::vector<SHalfTransition> mHalfTransitions;
    IMetaTransition *mpDefaultTransition;

public:
    CSourceAnimData(CResourceEntry *pEntry = 0)
        : CResource(pEntry)
        , mpDefaultTransition(nullptr)
    {}

    ~CSourceAnimData()
    {
        for (uint32 TransIdx = 0; TransIdx < mTransitions.size(); TransIdx++)
            delete mTransitions[TransIdx].pTransition;

        for (uint32 HalfIdx = 0; HalfIdx < mHalfTransitions.size(); HalfIdx++)
            delete mHalfTransitions[HalfIdx].pTransition;

        delete mpDefaultTransition;
    }

    CDependencyTree* BuildDependencyTree() const
    {
        // SAND normally has dependencies from meta-transitions and events
        // However, all of these can be character-specific. To simplify things, all SAND
        // dependencies are being added to the CHAR dependency tree instead. Therefore the
        // SAND dependency tree is left empty.
        return new CDependencyTree();
    }

    void GetUniquePrimitives(std::set<CAnimPrimitive>& rPrimSet) const
    {
        for (uint32 TransIdx = 0; TransIdx < mTransitions.size(); TransIdx++)
            mTransitions[TransIdx].pTransition->GetUniquePrimitives(rPrimSet);

        for (uint32 HalfIdx = 0; HalfIdx < mHalfTransitions.size(); HalfIdx++)
            mHalfTransitions[HalfIdx].pTransition->GetUniquePrimitives(rPrimSet);

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

            if (UsedTransitions.find(mpDefaultTransition) == UsedTransitions.end())
            {
                mpDefaultTransition->GetUniquePrimitives(PrimSet);
                UsedTransitions.insert(mpDefaultTransition);
            }

            for (uint32 TransitionIdx = 0; TransitionIdx < mTransitions.size(); TransitionIdx++)
            {
                const STransition& rkTransition = mTransitions[TransitionIdx];
                IMetaTransition *pTransition = rkTransition.pTransition;

                if ( pTree->HasDependency(rkTransition.AnimA) &&
                     pTree->HasDependency(rkTransition.AnimB) &&
                     UsedTransitions.find(pTransition) == UsedTransitions.end() )
                {
                    pTransition->GetUniquePrimitives(PrimSet);
                    UsedTransitions.insert(pTransition);
                }
            }

            for (uint32 HalfIdx = 0; HalfIdx < mHalfTransitions.size(); HalfIdx++)
            {
                const SHalfTransition& rkHalfTrans = mHalfTransitions[HalfIdx];
                IMetaTransition *pTransition = rkHalfTrans.pTransition;

                if ( pTree->HasDependency(rkHalfTrans.Anim) &&
                     UsedTransitions.find(pTransition) == UsedTransitions.end() )
                {
                    pTransition->GetUniquePrimitives(PrimSet);
                    UsedTransitions.insert(pTransition);
                }
            }

            // If we have no new primitives then we've exhausted all usable transitions; break out
            if (PrimSet.empty())
                break;

            // Add all transition primitives to the tree
            for (auto Iter = PrimSet.begin(); Iter != PrimSet.end(); Iter++)
                pTree->AddDependency(Iter->Animation());
        }
    }
};

#endif // CSOURCEANIMDATA_H
