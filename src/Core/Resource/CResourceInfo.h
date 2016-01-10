#ifndef CRESOURCEINFO
#define CRESOURCEINFO

#include "CResource.h"
#include "CResCache.h"
#include <Common/CUniqueID.h>
#include <Common/CFourCC.h>
#include <boost/filesystem.hpp>

class CResourceInfo
{
    TString mPath;
    bool mIsPath;
    bool mIsValidPath;

public:
    CResourceInfo()
        : mPath(""), mIsPath(false), mIsValidPath(false) {}

    CResourceInfo(const TString& rkPath)
        : mPath(rkPath), mIsPath(true)
    {
        mIsValidPath = boost::filesystem::exists(rkPath.ToStdString());
    }

    CResourceInfo(const CUniqueID& rkID, CFourCC Type)
        : mIsPath(false), mIsValidPath(false)
    {
        mPath = rkID.ToString() + "." + Type.ToString();
    }

    inline CUniqueID ID() const
    {
        if (!mIsPath)
            return CUniqueID::FromString(mPath.GetFileName());
        else
            return CUniqueID::skInvalidID64;
    }

    inline CFourCC Type() const
    {
        return mPath.GetFileExtension();
    }

    inline TString ToString() const
    {
        return mPath;
    }

    inline CResource* Load() const
    {
        if (!IsValid())
            return nullptr;
        if (mIsPath)
            return gResCache.GetResource(mPath);
        else
            return gResCache.GetResource(ID(), Type());
    }

    inline bool IsValid() const
    {
        if (!mIsPath)
            return ID().IsValid();
        else
            return mIsValidPath;
    }
};

#endif // CRESOURCEINFO

