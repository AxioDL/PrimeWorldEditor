#ifndef CANIMEVENTDATA
#define CANIMEVENTDATA

#include "CResource.h"

class CAnimEventData : public CResource
{
    struct SEvent
    {
        u32 mCharacterIndex;
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
        CDependencyTree *pTree = new CDependencyTree(ID());

        for (u32 iEvt = 0; iEvt < mEvents.size(); iEvt++)
        {
            const SEvent& rkEvent = mEvents[iEvt];
            pTree->AddEventDependency(rkEvent.mAssetRef, rkEvent.mCharacterIndex);
        }

        return pTree;
    }

    inline u32 NumEvents() const                        { return mEvents.size(); }
    inline u32 EventCharacterIndex(u32 EventIdx) const  { return mEvents[EventIdx].mCharacterIndex; }
    inline CAssetID EventAssetRef(u32 EventIdx) const   { return mEvents[EventIdx].mAssetRef; }

    inline void AddEvent(u32 CharIdx, CAssetID AssetID) { mEvents.push_back( SEvent { CharIdx, AssetID } ); }
};

#endif // CANIMEVENTDATA

