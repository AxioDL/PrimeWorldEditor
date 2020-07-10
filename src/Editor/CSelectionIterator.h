#ifndef CSELECTIONITERATOR
#define CSELECTIONITERATOR

#include "CNodeSelection.h"

class CSelectionIterator
{
    CNodeSelection *mpSelection;
    uint32 mCurrentIndex = 0;

public:
    explicit CSelectionIterator(CNodeSelection *pSelection)
        : mpSelection(pSelection) {}

    void Next()                          { mCurrentIndex++; }
    bool DoneIterating() const           { return (mCurrentIndex == mpSelection->Size()); }
    explicit operator bool() const       { return !DoneIterating(); }
    CSceneNode* operator*() const        { return mpSelection->At(mCurrentIndex); }
    CSceneNode* operator->() const       { return mpSelection->At(mCurrentIndex); }
    CSelectionIterator& operator++()     { Next(); return *this; }
    CSelectionIterator operator++(int)   { CSelectionIterator Copy = *this; Next(); return Copy; }
};

#endif // CSELECTIONITERATOR

