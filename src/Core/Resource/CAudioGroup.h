#ifndef CAUDIOGROUP
#define CAUDIOGROUP

#include "CResource.h"

// Very limited functionality - mostly just intended to find the AGSC that a sound ID belongs to
class CAudioGroup : public CResource
{
    DECLARE_RESOURCE_TYPE(eAudioGroup)
    friend class CAudioGroupLoader;

    TString mGroupName;
    u32 mGroupID;
    std::vector<u16> mDefineIDs;

public:
    CAudioGroup(CResourceEntry *pEntry = 0)
        : CResource(pEntry)
        , mGroupID(-1)
    {}

    // Accessors
    inline TString GroupName() const                    { return mGroupName; }
    inline u32 GroupID() const                          { return mGroupID; }
    inline u32 NumSoundDefineIDs() const                { return mDefineIDs.size(); }
    inline u16 SoundDefineIDByIndex(u32 Index) const    { return mDefineIDs[Index]; }
};

#endif // CAUDIOGROUP

