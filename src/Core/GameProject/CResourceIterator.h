#ifndef CRESOURCEITERATOR
#define CRESOURCEITERATOR

#include <Core/GameProject/CResourceEntry.h>
#include <Core/GameProject/CResourceStore.h>

class CResourceIterator
{
protected:
    CResourceStore *mpStore;
    std::map<CAssetID, CResourceEntry*>::iterator mIter;
    CResourceEntry *mpCurEntry;

public:
    CResourceIterator(CResourceStore *pStore = gpResourceStore)
        : mpStore(pStore)
        , mpCurEntry(nullptr)
    {
        mIter = mpStore->mResourceEntries.begin();
        Next();
    }

    virtual CResourceEntry* Next()
    {
        if (mIter != mpStore->mResourceEntries.end())
        {
            mpCurEntry = mIter->second;
            mIter++;
        }
        else mpCurEntry = nullptr;

        return mpCurEntry;
    }

    inline bool DoneIterating() const
    {
        return mpCurEntry == nullptr;
    }

    inline operator bool() const
    {
        return !DoneIterating();
    }

    inline CResourceEntry* operator*() const
    {
        return mpCurEntry;
    }

    inline CResourceEntry* operator->() const
    {
        return mpCurEntry;
    }

    inline CResourceIterator& operator++()
    {
        Next();
        return *this;
    }

    inline CResourceIterator operator++(int)
    {
        CResourceIterator Copy = *this;
        Next();
        return Copy;
    }
};

template<class ResType>
class TResourceIterator : public CResourceIterator
{
public:
    TResourceIterator(CResourceStore *pStore = gpResourceStore)
        : CResourceIterator(pStore)
    {
        if (mpCurEntry->ResourceType() != ResType::StaticType())
            Next();
    }

    virtual CResourceEntry* Next()
    {
        do {
            CResourceIterator::Next();
        } while (mpCurEntry && mpCurEntry->ResourceType() != ResType::StaticType());

        return mpCurEntry;
    }
};

#endif // CRESOURCEITERATOR

