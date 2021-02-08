#ifndef CCOLLISIONEDITORVIEWPORT_H
#define CCOLLISIONEDITORVIEWPORT_H

#include "Editor/CBasicViewport.h"
#include "Editor/CGridRenderable.h"
#include <Core/Scene/CCollisionNode.h>
#include <memory>

/** Preview viewport for the collision editor */
class CCollisionEditorViewport : public CBasicViewport
{
    Q_OBJECT

    std::unique_ptr<CRenderer>  mpRenderer;
    CCollisionNode*             mpCollisionNode = nullptr;
    CGridRenderable             mGrid;
    bool                        mGridEnabled = true;

public:
    /** Constructor */
    explicit CCollisionEditorViewport(QWidget* pParent = nullptr);

    /** CBasicViewport interface */
    void Paint() override;
    void OnResize() override;

    /** Accessors */
    void SetNode(CCollisionNode* pNode)      { mpCollisionNode = pNode; }
    void SetGridEnabled(bool Enabled)        { mGridEnabled = Enabled; }
    void SetOBBTreeDepth(int Depth)
    {
        mViewInfo.CollisionSettings.DrawBoundingHierarchy = (Depth > 0);
        mViewInfo.CollisionSettings.BoundingHierarchyRenderDepth = Depth;
    }
};

#endif // CCOLLISIONEDITORVIEWPORT_H
