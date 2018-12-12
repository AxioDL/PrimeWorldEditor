#ifndef CSELECTIONITERATOR
#define CSELECTIONITERATOR

#include "CNodeSelection.h"

class CSelectionIterator
{
    CNodeSelection *mpSelection;
    uint32 mCurrentIndex;

public:
    CSelectionIterator(CNodeSelection *pSelection)
        : mpSelection(pSelection), mCurrentIndex(0) {}

    inline void Next()                          { mCurrentIndex++; }
    inline bool DoneIterating() const           { return (mCurrentIndex == mpSelection->Size()); }
    inline operator bool() const                { return !DoneIterating(); }
    inline CSceneNode* operator*() const        { return mpSelection->At(mCurrentIndex); }
    inline CSceneNode* operator->() const       { return mpSelection->At(mCurrentIndex); }
    inline CSelectionIterator& operator++()     { Next(); return *this; }
    inline CSelectionIterator operator++(int)   { CSelectionIterator Copy = *this; Next(); return Copy; }
};

#endif // CSELECTIONITERATOR

