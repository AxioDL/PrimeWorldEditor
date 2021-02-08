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

    CSceneNode* Next()
    {
        InternalFindNext();
        return mpCurNode;
    }

    bool DoneIterating() const
    {
        return (mpCurNode == nullptr);
    }

    CSceneNode* GetNode() const
    {
        return mpCurNode;
    }

    explicit operator bool() const
    {
        return !DoneIterating();
    }

    CSceneNode* operator*() const
    {
        return mpCurNode;
    }

    CSceneNode* operator->() const
    {
        return mpCurNode;
    }

    CSceneIterator& operator++()
    {
        Next();
        return *this;
    }

    CSceneIterator operator++(int)
    {
        CSceneIterator Copy = *this;
        Next();
        return Copy;
    }

protected:
    void InternalFindNext();
};

#endif // CSCENEITERATOR_H
