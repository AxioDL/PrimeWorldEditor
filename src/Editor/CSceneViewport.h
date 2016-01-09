#ifndef CSCENEVIEWPORT_H
#define CSCENEVIEWPORT_H

#include "CBasicViewport.h"
#include "INodeEditor.h"

class CSceneViewport : public CBasicViewport
{
    Q_OBJECT

    INodeEditor *mpEditor;
    CScene *mpScene;
    CRenderer *mpRenderer;

    // Scene interaction
    bool mGizmoHovering;
    bool mGizmoTransforming;
    CSceneNode *mpHoverNode;
    CVector3f mHoverPoint;

    // Context Menu
    QMenu *mpContextMenu;
    QAction *mpToggleSelectAction;
    QAction *mpHideSelectionSeparator;
    QAction *mpHideSelectionAction;
    QAction *mpHideUnselectedAction;
    QAction *mpHideHoverSeparator;
    QAction *mpHideHoverNodeAction;
    QAction *mpHideHoverTypeAction;
    QAction *mpHideHoverLayerAction;
    QAction *mpUnhideSeparator;
    QAction *mpUnhideAllAction;
    CSceneNode *mpMenuNode;

public:
    CSceneViewport(QWidget *pParent = 0);
    ~CSceneViewport();
    void SetScene(INodeEditor *pEditor, CScene *pScene);
    void SetShowFlag(EShowFlag Flag, bool Visible);
    FShowFlags ShowFlags() const;
    CRenderer* Renderer();
    CSceneNode* HoverNode();
    CVector3f HoverPoint();
    void CheckGizmoInput(const CRay& ray);
    void SceneRayCast(const CRay& ray);
    void ResetHover();
    bool IsHoveringGizmo();

    void keyPressEvent(QKeyEvent* pEvent);
    void keyReleaseEvent(QKeyEvent* pEvent);

protected:
    void CreateContextMenu();

signals:
    void GizmoMoved();
    void CameraOrbit();

protected slots:
    void CheckUserInput();
    void Paint();
    void ContextMenu(QContextMenuEvent *pEvent);
    void OnResize();
    void OnMouseClick(QMouseEvent *pEvent);
    void OnMouseRelease(QMouseEvent *pEvent);

    // Menu Actions
    void OnToggleSelect();
    void OnHideSelection();
    void OnHideUnselected();
    void OnHideNode();
    void OnHideType();
    void OnHideLayer();
    void OnUnhideAll();
    void OnContextMenuClose();
};

#endif // CSCENEVIEWPORT_H
