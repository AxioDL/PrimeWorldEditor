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
        AddDependenciesToTree(pTree);
        return pTree;
    }

    void AddDependenciesToTree(CDependencyTree *pTree) const
    {
        for (u32 iEvt = 0; iEvt < mEvents.size(); iEvt++)
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

    inline u32 NumEvents() const                        { return mEvents.size(); }
    inline u32 EventCharacterIndex(u32 EventIdx) const  { return mEvents[EventIdx].mCharacterIndex; }
    inline CAssetID EventAssetRef(u32 EventIdx) const   { return mEvents[EventIdx].mAssetRef; }

    inline void AddEvent(u32 CharIdx, CAssetID AssetID) { mEvents.push_back( SEvent { CharIdx, AssetID } ); }
};

#endif // CANIMEVENTDATA

