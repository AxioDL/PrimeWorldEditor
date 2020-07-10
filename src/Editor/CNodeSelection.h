#ifndef CNODESELECTION_H
#define CNODESELECTION_H

#include <Core/Scene/CSceneIterator.h>
#include <Core/Scene/CSceneNode.h>
#include <Core/Scene/CScriptNode.h>
#include <QList>
#include <QObject>
#include <QSignalBlocker>

class CNodeSelection : public QObject
{
    Q_OBJECT

    FNodeFlags mAllowedNodes{ENodeType::All};
    QList<CSceneNode*> mSelectedNodes;

    mutable CAABox mCachedBounds;
    mutable bool mBoundsDirty = true;

public:
    CNodeSelection() = default;

    ~CNodeSelection() override
    {
        for (CSceneNode *pNode : mSelectedNodes)
            pNode->SetSelected(false);
    }

    void SelectNode(CSceneNode *pNode)
    {
        if (IsAllowedType(pNode->NodeType()) && !pNode->IsSelected())
        {
            pNode->SetSelected(true);
            mSelectedNodes.push_back(pNode);
            mCachedBounds.ExpandBounds(pNode->AABox());
            emit Modified();
        }
    }

    void DeselectNode(CSceneNode *pNode)
    {
        if (pNode->IsSelected())
        {
            pNode->SetSelected(false);
            mSelectedNodes.removeOne(pNode);
            mBoundsDirty = true;
            emit Modified();
        }
    }

    void Clear()
    {
        for (CSceneNode *pNode : mSelectedNodes)
            pNode->SetSelected(false);

        mSelectedNodes.clear();
        mBoundsDirty = true;
        emit Modified();
    }

    void ClearAndSelectNode(CSceneNode *pNode)
    {
        // Block signals for Clear so that Modified only emits once.
        {
            [[maybe_unused]] const QSignalBlocker blocker{this};
            Clear();
        }

        SelectNode(pNode);
    }

    void SetSelectedNodes(const QList<CSceneNode*>& rkList)
    {
        {
            [[maybe_unused]] const QSignalBlocker blocker{this};
            Clear();

            for (CSceneNode* pNode : rkList)
                SelectNode(pNode);
        }

        mBoundsDirty = true;
        emit Modified();
    }

    CAABox Bounds() const
    {
        if (mBoundsDirty)
        {
            mCachedBounds = CAABox::Infinite();

            for (CSceneNode *pNode : mSelectedNodes)
            {
                mCachedBounds.ExpandBounds(pNode->AABox());

                if (pNode->NodeType() == ENodeType::Script)
                {
                    CScriptNode *pScript = static_cast<CScriptNode*>(pNode);

                    if (pScript->HasPreviewVolume())
                        mCachedBounds.ExpandBounds(pScript->PreviewVolumeAABox());
                }
            }

            mBoundsDirty = false;
        }

        return mCachedBounds;
    }

    uint32 Size() const                          { return mSelectedNodes.size(); }
    bool IsEmpty() const                         { return Size() == 0; }
    CSceneNode* At(uint32 Index) const           { return mSelectedNodes[Index]; }
    CSceneNode* Front() const                    { return mSelectedNodes.front(); }
    CSceneNode* Back() const                     { return mSelectedNodes.back(); }
    CSceneNode* operator[](uint32 Index) const   { return mSelectedNodes[Index]; }
    void UpdateBounds()                          { mBoundsDirty = true; }
    void SetAllowedNodeTypes(FNodeFlags Types)   { mAllowedNodes = Types; }
    bool IsAllowedType(ENodeType Type) const     { return (mAllowedNodes & Type) != 0; }
    bool IsAllowedType(CSceneNode *pNode) const  { return (mAllowedNodes & pNode->NodeType()) != 0; }
    QList<CSceneNode*> SelectedNodeList() const  { return mSelectedNodes; }

signals:
    void Modified();
};

#endif // CNODESELECTION_H
