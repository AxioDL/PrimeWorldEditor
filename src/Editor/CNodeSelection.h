#ifndef CNODESELECTION_H
#define CNODESELECTION_H

#include <Common/Math/CAABox.h>
#include <Core/Scene/CSceneNode.h>
#include <Core/Scene/CScriptNode.h>
#include <QList>
#include <QObject>
#include <QSignalBlocker>

#include <span>

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

    [[nodiscard]] CAABox Bounds() const
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

    [[nodiscard]] std::span<CSceneNode*> Nodes()             { return mSelectedNodes; }
    [[nodiscard]] std::span<CSceneNode* const> Nodes() const { return mSelectedNodes; }

    [[nodiscard]] uint32_t Size() const              { return mSelectedNodes.size(); }
    [[nodiscard]] bool IsEmpty() const               { return Size() == 0; }
    [[nodiscard]] bool SingleNodeSelected() const    { return Size() == 1; }
    [[nodiscard]] bool MultipleNodesSelected() const { return Size() >= 2; }
    [[nodiscard]] CSceneNode* Front() const          { return mSelectedNodes.front(); }
    [[nodiscard]] CSceneNode* Back() const           { return mSelectedNodes.back(); }

    void UpdateBounds()                        { mBoundsDirty = true; }
    void SetAllowedNodeTypes(FNodeFlags Types) { mAllowedNodes = Types; }

    [[nodiscard]] bool IsAllowedType(ENodeType Type) const           { return mAllowedNodes.HasFlag(Type); }
    [[nodiscard]] bool IsAllowedType(const CSceneNode *pNode) const  { return mAllowedNodes.HasFlag(pNode->NodeType()); }
    [[nodiscard]] const QList<CSceneNode*>& SelectedNodeList() const { return mSelectedNodes; }

signals:
    void Modified();
};

#endif // CNODESELECTION_H
