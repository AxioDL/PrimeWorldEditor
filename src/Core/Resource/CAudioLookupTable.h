#ifndef CAUDIOLOOKUPTABLE
#define CAUDIOLOOKUPTABLE

#include "CResource.h"
#include <vector>

class CAudioLookupTable : public CResource
{
    DECLARE_RESOURCE_TYPE(AudioLookupTable)
    friend class CAudioGroupLoader;
    std::vector<uint16> mDefineIDs;

public:
    explicit CAudioLookupTable(CResourceEntry *pEntry = nullptr)
        : CResource(pEntry)
    {}

    uint16 FindSoundDefineID(size_t SoundID) const
    {
        if (SoundID >= mDefineIDs.size()) 
            return UINT16_MAX;

        return mDefineIDs[SoundID];
    }
};

#endif // CAUDIOLOOKUPTABLE

