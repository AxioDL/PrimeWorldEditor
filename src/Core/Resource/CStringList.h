#ifndef CSTRINGLIST
#define CSTRINGLIST

#include "CResource.h"

class CStringList : public CResource
{
    DECLARE_RESOURCE_TYPE(StringList)
    friend class CAudioGroupLoader;
    std::vector<TString> mStringList;

public:
    explicit CStringList(CResourceEntry *pEntry = nullptr)
        : CResource(pEntry)
    {}

    size_t NumStrings() const
    {
        return mStringList.size();
    }

    TString StringByIndex(size_t Index) const
    {
        ASSERT(Index < mStringList.size());
        return mStringList[Index];
    }
};

#endif // CSTRINGLIST

