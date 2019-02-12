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

public:
    /** Constructor */
    CCollisionEditorViewport(QWidget* pParent = 0);

    /** Update the collision node to render in the scene */
    void SetNode(CCollisionNode* pNode);

    /** CBasicViewport interface */
    virtual void Paint() override;
    virtual void OnResize() override;
};

#endif // CCOLLISIONEDITORVIEWPORT_H
