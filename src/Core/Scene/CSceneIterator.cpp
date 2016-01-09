#include "CSceneIterator.h"
#include "CScene.h"

CSceneIterator::CSceneIterator(CScene *pScene, FNodeFlags AllowedNodeTypes /*= eAllNodeTypes*/, bool AllowHiddenNodes /*= false*/)
    : mpScene(pScene)
    , mAllowHiddenNodes(AllowHiddenNodes)
    , mNodeFlags(AllowedNodeTypes)
    , mpCurNode(nullptr)
{
    mMapIterator = mpScene->mNodes.begin();

    while (mMapIterator != mpScene->mNodes.end())
    {
        if (mMapIterator->first & AllowedNodeTypes)
            break;

        mMapIterator++;
    }

    if (mMapIterator != mpScene->mNodes.end())
    {
        mVectorIterator = (mMapIterator->second).begin();
        Next(); // Find first node
    }
}

// ************ PROTECTED ************
void CSceneIterator::InternalFindNext()
{
    // This function does most of the heavy lifting. We continue from where we left off last time this function was called.
    while (mMapIterator != mpScene->mNodes.end())
    {
        // Iterate over each node in the vector.
        std::vector<CSceneNode*>& rVector = mMapIterator->second;
        bool FoundNext = false;

        while (mVectorIterator != rVector.end())
        {
            CSceneNode *pNode = *mVectorIterator;

            // Check node visibility
            if (mAllowHiddenNodes || pNode->IsVisible())
            {
                mpCurNode = pNode;
                FoundNext = true;
            }

            mVectorIterator++;
            if (FoundNext) return;
        }

        // We've reached the end of this node vector, so advance the map iterator
        while (true)
        {
            mMapIterator++;

            if (mMapIterator == mpScene->mNodes.end())
            {
                break;
            }

            if (mNodeFlags & mMapIterator->first)
            {
                mVectorIterator = mMapIterator->second.begin();
                break;
            }
        }
    }

    // If we're down here, then it seems we're done iterating the scene.
    mpCurNode = nullptr;
}
