#ifndef CPACKAGE
#define CPACKAGE

#include <Common/CFourCC.h>
#include <Common/CUniqueID.h>
#include <Common/TString.h>

struct SNamedResource
{
    TString Name;
    CUniqueID ID;
};

class CPackage
{
    TString mPakName;
    std::vector<SNamedResource> mNamedResources;
    std::vector<CUniqueID> mPakResources;

public:
    CPackage() {}
    CPackage(const TString& rkName) : mPakName(rkName) {}

    inline TString PakName() const                                      { return mPakName; }
    inline u32 NumNamedResources() const                                { return mNamedResources.size(); }
    inline const SNamedResource& NamedResourceByIndex(u32 Index) const  { return mNamedResources[Index]; }

    inline void SetPakName(TString NewName)                             { mPakName = NewName; }
    inline void AddNamedResource(TString Name, const CUniqueID& rkID)   { mNamedResources.push_back( SNamedResource { Name, rkID } ); }
    inline void AddPakResource(const CUniqueID& rkID)                   { mPakResources.push_back(rkID); }
};

#endif // CPACKAGE

