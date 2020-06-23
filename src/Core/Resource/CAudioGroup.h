#ifndef CAUDIOGROUP
#define CAUDIOGROUP

#include "CResource.h"

// Very limited functionality - mostly just intended to find the AGSC that a sound ID belongs to
class CAudioGroup : public CResource
{
    DECLARE_RESOURCE_TYPE(AudioGroup)
    friend class CAudioGroupLoader;

    TString mGroupName;
    uint32 mGroupID = UINT32_MAX;
    std::vector<uint16> mDefineIDs;

public:
    explicit CAudioGroup(CResourceEntry *pEntry = nullptr)
        : CResource(pEntry)
    {}

    // Accessors
    TString GroupName() const                        { return mGroupName; }
    uint32 GroupID() const                           { return mGroupID; }
    size_t NumSoundDefineIDs() const                 { return mDefineIDs.size(); }
    uint16 SoundDefineIDByIndex(size_t Index) const  { return mDefineIDs[Index]; }
};

#endif // CAUDIOGROUP

