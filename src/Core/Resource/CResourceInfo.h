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
        : mPath("FFFFFFFFFFFFFFFF"), mIsPath(false), mIsValidPath(false) {}

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
        TString FileName = mPath.GetFileName(false);

        if (!mIsPath)
            return CUniqueID::FromString(FileName);

        else
        {
            if (FileName.IsHexString() && (FileName.Size() == 8 || FileName.Size() == 16))
                return CUniqueID::FromString(FileName);
            else
                return CUniqueID::skInvalidID64;
        }
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

    inline bool operator==(const CResourceInfo& rkOther) const
    {
        return (mPath.GetFileName() == rkOther.mPath.GetFileName());
    }

    inline bool operator!=(const CResourceInfo& rkOther) const
    {
        return (!(*this == rkOther));
    }
};

#endif // CRESOURCEINFO

