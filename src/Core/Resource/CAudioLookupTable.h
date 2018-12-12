#ifndef CAUDIOLOOKUPTABLE
#define CAUDIOLOOKUPTABLE

#include "CResource.h"

class CAudioLookupTable : public CResource
{
    DECLARE_RESOURCE_TYPE(eAudioLookupTable)
    friend class CAudioGroupLoader;
    std::vector<uint16> mDefineIDs;

public:
    CAudioLookupTable(CResourceEntry *pEntry = 0)
        : CResource(pEntry)
    {}

    inline uint16 FindSoundDefineID(uint32 SoundID)
    {
        if (SoundID >= mDefineIDs.size()) return -1;
        return mDefineIDs[SoundID];
    }
};

#endif // CAUDIOLOOKUPTABLE

