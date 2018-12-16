#ifndef CNODESELECTION_H
#define CNODESELECTION_H

#include <Core/Scene/CSceneIterator.h>
#include <Core/Scene/CSceneNode.h>
#include <Core/Scene/CScriptNode.h>
#include <QList>
#include <QObject>

class CNodeSelection : public QObject
{
    Q_OBJECT

    FNodeFlags mAllowedNodes;
    QList<CSceneNode*> mSelectedNodes;

    mutable CAABox mCachedBounds;
    mutable bool mBoundsDirty;

public:
    CNodeSelection()
        : mAllowedNodes(ENodeType::All)
        , mBoundsDirty(true) {}

    ~CNodeSelection()
    {
        foreach (CSceneNode *pNode, mSelectedNodes)
            pNode->SetSelected(false);
    }

    void SelectNode(CSceneNode *pNode)
    {
        if (IsAllowedType(pNode->NodeType()) && !pNode->IsSelected())
        {
            pNode->SetSelected(true);
            mSelectedNodes << pNode;
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
        foreach (CSceneNode *pNode, mSelectedNodes)
            pNode->SetSelected(false);

        mSelectedNodes.clear();
        mBoundsDirty = true;
        emit Modified();
    }

    void ClearAndSelectNode(CSceneNode *pNode)
    {
        // Block signals for Clear so that Modified only emits once.
        blockSignals(true);
        Clear();
        blockSignals(false);
        SelectNode(pNode);
    }

    void SetSelectedNodes(const QList<CSceneNode*>& rkList)
    {
        blockSignals(true);
        Clear();

        foreach (CSceneNode *pNode, rkList)
            SelectNode(pNode);
        blockSignals(false);

        mBoundsDirty = true;
        emit Modified();
    }

    CAABox Bounds() const
    {
        if (mBoundsDirty)
        {
            mCachedBounds = CAABox::skInfinite;

            foreach (CSceneNode *pNode, mSelectedNodes)
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

    inline uint32 Size() const                          { return mSelectedNodes.size(); }
    inline bool IsEmpty() const                         { return Size() == 0; }
    inline CSceneNode* At(uint32 Index) const           { return mSelectedNodes[Index]; }
    inline CSceneNode* Front() const                    { return mSelectedNodes.front(); }
    inline CSceneNode* Back() const                     { return mSelectedNodes.back(); }
    inline CSceneNode* operator[](uint32 Index) const   { return mSelectedNodes[Index]; }
    inline void UpdateBounds()                          { mBoundsDirty = true; }
    inline void SetAllowedNodeTypes(FNodeFlags Types)   { mAllowedNodes = Types; }
    inline bool IsAllowedType(ENodeType Type) const     { return (mAllowedNodes & Type) != 0; }
    inline bool IsAllowedType(CSceneNode *pNode) const  { return (mAllowedNodes & pNode->NodeType()) != 0; }
    inline QList<CSceneNode*> SelectedNodeList() const  { return mSelectedNodes; }

signals:
    void Modified();
};

#endif // CNODESELECTION_H
