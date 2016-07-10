#ifndef CRESOURCEITERATOR
#define CRESOURCEITERATOR

#include <Core/GameProject/CResourceEntry.h>
#include <Core/GameProject/CResourceStore.h>

class CResourceIterator
{
    CResourceStore *mpStore;
    std::map<CUniqueID, CResourceEntry*>::iterator mIter;
    CResourceEntry *mpCurEntry;

public:
    CResourceIterator(CResourceStore *pStore)
        : mpStore(pStore)
        , mpCurEntry(nullptr)
    {
        mIter = mpStore->mResourceEntries.begin();
        Next();
    }

    inline CResourceEntry* Next()
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

#endif // CRESOURCEITERATOR

