#ifndef CANIMEVENTLOADER_H
#define CANIMEVENTLOADER_H

#include "Core/Resource/CAnimEventData.h"
#include "Core/Resource/TResPtr.h"

class CAnimEventLoader
{
    TResPtr<CAnimEventData> mpEventData;

    CAnimEventLoader() {}
    void LoadEvents(IInputStream& rEVNT, bool IsEchoes);

public:
    static CAnimEventData* LoadEVNT(IInputStream& rEVNT, CResourceEntry *pEntry);
    static CAnimEventData* LoadAnimSetEvents(IInputStream& rANCS);
};

#endif // CANIMEVENTLOADER_H
