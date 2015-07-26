#ifndef CRESOURCE_H
#define CRESOURCE_H

#include "EResType.h"
#include <Common/CFourCC.h>
#include <Common/CUniqueID.h>
#include <Common/types.h>
#include <Common/StringUtil.h>

class CResCache;

class CResource
{
    friend class CResCache;

    std::string mResSource;
    CUniqueID mID;
    int mRefCount;

public:
    CResource();
    virtual ~CResource();
    virtual EResType Type();
    std::string Source();
    std::string FullSource();
    CUniqueID ResID();
    void Lock();
    void Release();
    bool IsValidResource();
    static EResType ResTypeForExtension(CFourCC Extension);
};

#endif // CRESOURCE_H
