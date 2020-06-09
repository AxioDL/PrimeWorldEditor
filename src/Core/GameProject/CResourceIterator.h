#ifndef CRESOURCEITERATOR
#define CRESOURCEITERATOR

#include <Core/GameProject/CResourceEntry.h>
#include <Core/GameProject/CResourceStore.h>

class CResourceIterator
{
protected:
    const CResourceStore *mpkStore;
    std::map<CAssetID, CResourceEntry*>::const_iterator mIter;
    CResourceEntry *mpCurEntry;

public:
    CResourceIterator(const CResourceStore *pkStore = gpResourceStore)
        : mpkStore(pkStore)
        , mpCurEntry(nullptr)
    {
        mIter = mpkStore->mResourceEntries.begin();
        Next();
    }

    virtual CResourceEntry* Next()
    {
        do
        {
            if (mIter != mpkStore->mResourceEntries.end())
            {
                mpCurEntry = mIter->second;
                ++mIter;
            }
            else mpCurEntry = nullptr;
        }
        while (mpCurEntry && mpCurEntry->IsMarkedForDeletion());

        return mpCurEntry;
    }

    bool DoneIterating() const
    {
        return mpCurEntry == nullptr;
    }

    explicit operator bool() const
    {
        return !DoneIterating();
    }

    CResourceEntry* operator*() const
    {
        return mpCurEntry;
    }

    CResourceEntry* operator->() const
    {
        return mpCurEntry;
    }

    CResourceIterator& operator++()
    {
        Next();
        return *this;
    }

    CResourceIterator operator++(int)
    {
        CResourceIterator Copy = *this;
        Next();
        return Copy;
    }
};

template<EResourceType ResType>
class TResourceIterator : public CResourceIterator
{
public:
    TResourceIterator(CResourceStore *pStore = gpResourceStore)
        : CResourceIterator(pStore)
    {
        if (mpCurEntry && mpCurEntry->ResourceType() != ResType)
            Next();
    }

    CResourceEntry* Next() override
    {
        do {
            CResourceIterator::Next();
        } while (mpCurEntry && mpCurEntry->ResourceType() != ResType);

        return mpCurEntry;
    }
};

#endif // CRESOURCEITERATOR

