#ifndef CAUDIOGROUP
#define CAUDIOGROUP

#include "CResource.h"

// Very limited functionality - mostly just intended to find the AGSC that a sound ID belongs to
class CAudioGroup : public CResource
{
    DECLARE_RESOURCE_TYPE(AudioGroup)
    friend class CAudioGroupLoader;

    TString mGroupName;
    uint32 mGroupID;
    std::vector<uint16> mDefineIDs;

public:
    CAudioGroup(CResourceEntry *pEntry = 0)
        : CResource(pEntry)
        , mGroupID(-1)
    {}

    // Accessors
    inline TString GroupName() const                        { return mGroupName; }
    inline uint32 GroupID() const                           { return mGroupID; }
    inline uint32 NumSoundDefineIDs() const                 { return mDefineIDs.size(); }
    inline uint16 SoundDefineIDByIndex(uint32 Index) const  { return mDefineIDs[Index]; }
};

#endif // CAUDIOGROUP

