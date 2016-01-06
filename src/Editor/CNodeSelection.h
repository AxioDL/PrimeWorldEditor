#ifndef CNODESELECTION_H
#define CNODESELECTION_H

#include <Core/Scene/CSceneNode.h>
#include <list>

class CSceneSelection
{
    CScene *mpScene;
    std::vector<CSceneNode*> mSelectedNodes;

public:
    CSceneSelection(CScene *pScene);
    void SelectNode(CSceneNode *pNode);
    void DeselectNode(CSceneNode *pNode);
    u32 SelectionSize();
    CSceneNode* NodeByIndex(u32 Index);
    void ClearSelection();

    // Operators
    CSceneNode* operator[](u32 Index);
};

#endif // CNODESELECTION_H
