#ifndef CANIMEVENTDATA
#define CANIMEVENTDATA

#include "Core/Resource/CResource.h"

class CAnimEventData : public CResource
{
    DECLARE_RESOURCE_TYPE(AnimEventData)

    struct SEvent
    {
        uint32 mCharacterIndex;
        CAssetID mAssetRef;
    };

    std::vector<SEvent> mEvents;

public:
    explicit CAnimEventData(CResourceEntry *pEntry = nullptr)
        : CResource(pEntry)
    {
    }

    CDependencyTree* BuildDependencyTree() const override
    {
        auto *pTree = new CDependencyTree();
        AddDependenciesToTree(pTree);
        return pTree;
    }

    void AddDependenciesToTree(CDependencyTree *pTree) const
    {
        for (const SEvent& event : mEvents)
        {
            CAssetID ID = event.mAssetRef;

            if (ID.IsValid() && !pTree->HasDependency(ID))
            {
                auto *pDep = new CAnimEventDependency(ID, event.mCharacterIndex);
                pTree->AddChild(pDep);
            }
        }
    }

    uint32 NumEvents() const                             { return mEvents.size(); }
    uint32 EventCharacterIndex(uint32 EventIdx) const    { return mEvents[EventIdx].mCharacterIndex; }
    CAssetID EventAssetRef(uint32 EventIdx) const        { return mEvents[EventIdx].mAssetRef; }

    void AddEvent(uint32 CharIdx, CAssetID AssetID)      { mEvents.push_back(SEvent{CharIdx, AssetID}); }
};

#endif // CANIMEVENTDATA

