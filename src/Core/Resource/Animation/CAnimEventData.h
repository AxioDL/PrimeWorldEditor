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
    CAnimEventData(CResourceEntry *pEntry = 0)
        : CResource(pEntry)
    {
    }

    CDependencyTree* BuildDependencyTree() const
    {
        CDependencyTree *pTree = new CDependencyTree();
        AddDependenciesToTree(pTree);
        return pTree;
    }

    void AddDependenciesToTree(CDependencyTree *pTree) const
    {
        for (uint32 iEvt = 0; iEvt < mEvents.size(); iEvt++)
        {
            const SEvent& rkEvent = mEvents[iEvt];
            CAssetID ID = rkEvent.mAssetRef;

            if (ID.IsValid() && !pTree->HasDependency(ID))
            {
                CAnimEventDependency *pDep = new CAnimEventDependency(ID, rkEvent.mCharacterIndex);
                pTree->AddChild(pDep);
            }
        }
    }

    inline uint32 NumEvents() const                             { return mEvents.size(); }
    inline uint32 EventCharacterIndex(uint32 EventIdx) const    { return mEvents[EventIdx].mCharacterIndex; }
    inline CAssetID EventAssetRef(uint32 EventIdx) const        { return mEvents[EventIdx].mAssetRef; }

    inline void AddEvent(uint32 CharIdx, CAssetID AssetID)      { mEvents.push_back( SEvent { CharIdx, AssetID } ); }
};

#endif // CANIMEVENTDATA

