#include "CNodeSelection.h"

CSceneSelection::CSceneSelection(CScene *pScene)
{
    mpScene = pScene;
}

void CSceneSelection::SelectNode(CSceneNode *pNode)
{
    // There shouldn't be more than one selection per scene, so this should be safe.
    if (!pNode->IsSelected())
    {
        pNode->SetSelected(true);
        mSelectedNodes.push_back(pNode);
    }
}

void CSceneSelection::DeselectNode(CSceneNode *pNode)
{
    if (pNode->IsSelected())
    {
        pNode->SetSelected(false);

        for (auto it = mSelectedNodes.begin(); it != mSelectedNodes.end(); it++)
        {
            if (*it == pNode)
            {
                mSelectedNodes.erase(it);
                break;
            }
        }
    }
}

u32 CSceneSelection::SelectionSize()
{
    return mSelectedNodes.size();
}

CSceneNode* CSceneSelection::NodeByIndex(u32 Index)
{
    if (Index >= SelectionSize()) return nullptr;
    return mSelectedNodes[Index];
}

void CSceneSelection::ClearSelection()
{
    for (auto it = mSelectedNodes.begin(); it != mSelectedNodes.end(); it++)
        (*it)->SetSelected(false);

    mSelectedNodes.clear();
}

// ************ OPERATORS ************
CSceneNode* CSceneSelection::operator[](u32 Index)
{
    return NodeByIndex(Index);
}
