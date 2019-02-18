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
    CCollisionNode*             mpCollisionNode;
    CGridRenderable             mGrid;
    bool                        mGridEnabled;

public:
    /** Constructor */
    CCollisionEditorViewport(QWidget* pParent = 0);

    /** CBasicViewport interface */
    virtual void Paint() override;
    virtual void OnResize() override;

    /** Accessors */
    inline void SetNode(CCollisionNode* pNode)      { mpCollisionNode = pNode; }
    inline void SetGridEnabled(bool Enabled)        { mGridEnabled = Enabled; }
    inline void SetOBBTreeDepth(int Depth)
    {
        mViewInfo.CollisionSettings.DrawBoundingHierarchy = (Depth > 0);
        mViewInfo.CollisionSettings.BoundingHierarchyRenderDepth = Depth;
    }
};

#endif // CCOLLISIONEDITORVIEWPORT_H
