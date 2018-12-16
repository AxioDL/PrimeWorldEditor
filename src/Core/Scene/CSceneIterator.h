#ifndef CSCENEITERATOR_H
#define CSCENEITERATOR_H

#include "ENodeType.h"
#include <unordered_map>

class CScene;
class CSceneNode;

class CSceneIterator
{
    CScene *mpScene;
    bool mAllowHiddenNodes;
    FNodeFlags mNodeFlags;

    CSceneNode *mpCurNode;
    std::unordered_map<ENodeType, std::vector<CSceneNode*>>::iterator mMapIterator;
    std::vector<CSceneNode*>::iterator mVectorIterator;

public:
    CSceneIterator(CScene *pScene, FNodeFlags AllowedNodeTypes = ENodeType::All, bool AllowHiddenNodes = false);

    inline CSceneNode* Next()
    {
        InternalFindNext();
        return mpCurNode;
    }

    inline bool DoneIterating() const
    {
        return (mpCurNode == nullptr);
    }

    inline CSceneNode* GetNode() const
    {
        return mpCurNode;
    }

    inline operator bool() const
    {
        return !DoneIterating();
    }

    inline CSceneNode* operator*() const
    {
        return mpCurNode;
    }

    inline CSceneNode* operator->() const
    {
        return mpCurNode;
    }

    inline CSceneIterator& operator++()
    {
        Next();
        return *this;
    }

    inline CSceneIterator operator++(int)
    {
        CSceneIterator Copy = *this;
        Next();
        return Copy;
    }

protected:
    void InternalFindNext();
};

#endif // CSCENEITERATOR_H
