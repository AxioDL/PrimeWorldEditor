#ifndef CAUDIOLOOKUPTABLE
#define CAUDIOLOOKUPTABLE

#include "CResource.h"

class CAudioLookupTable : public CResource
{
    DECLARE_RESOURCE_TYPE(eAudioLookupTable)
    friend class CAudioGroupLoader;
    std::vector<u16> mDefineIDs;

public:
    CAudioLookupTable(CResourceEntry *pEntry = 0)
        : CResource(pEntry)
    {}

    inline u16 FindSoundDefineID(u32 SoundID)
    {
        if (SoundID >= mDefineIDs.size()) return -1;
        return mDefineIDs[SoundID];
    }
};

#endif // CAUDIOLOOKUPTABLE

