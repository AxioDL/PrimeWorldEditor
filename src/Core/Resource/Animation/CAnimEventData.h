#ifndef CANIMEVENTDATA
#define CANIMEVENTDATA

#include "Core/Resource/CResource.h"
#include <memory>
#include <vector>

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

    std::unique_ptr<CDependencyTree> BuildDependencyTree() const override
    {
        auto pTree = std::make_unique<CDependencyTree>();
        AddDependenciesToTree(pTree.get());
        return pTree;
    }

    void AddDependenciesToTree(CDependencyTree *pTree) const
    {
        for (const SEvent& event : mEvents)
        {
            CAssetID ID = event.mAssetRef;

            if (ID.IsValid() && !pTree->HasDependency(ID))
            {
                pTree->AddChild(std::make_unique<CAnimEventDependency>(ID, event.mCharacterIndex));
            }
        }
    }

    size_t NumEvents() const                             { return mEvents.size(); }
    uint32 EventCharacterIndex(size_t EventIdx) const    { return mEvents[EventIdx].mCharacterIndex; }
    CAssetID EventAssetRef(size_t EventIdx) const        { return mEvents[EventIdx].mAssetRef; }

    void AddEvent(uint32 CharIdx, CAssetID AssetID)      { mEvents.push_back(SEvent{CharIdx, AssetID}); }
};

#endif // CANIMEVENTDATA

