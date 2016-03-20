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
    bool mRenderingMergedWorld;

    // Scene interaction
    bool mGizmoHovering;
    bool mGizmoTransforming;
    SRayIntersection mRayIntersection;
    CSceneNode *mpHoverNode;
    CVector3f mHoverPoint;

    // Context Menu
    QMenu *mpContextMenu;
    QAction *mpToggleSelectAction;
    QAction *mpSelectConnectedAction;
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

    // Link Line
    bool mLinkLineEnabled;
    CVector3f mLinkLinePoints[2];

public:
    CSceneViewport(QWidget *pParent = 0);
    ~CSceneViewport();
    void SetScene(INodeEditor *pEditor, CScene *pScene);
    void SetShowFlag(EShowFlag Flag, bool Visible);
    void SetShowWorld(bool Visible);
    void SetRenderMergedWorld(bool b);
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

    inline void SetLinkLineEnabled(bool Enable) { mLinkLineEnabled = Enable; }

    inline void SetLinkLine(const CVector3f& rkPointA, const CVector3f& rkPointB)
    {
        mLinkLinePoints[0] = rkPointA;
        mLinkLinePoints[1] = rkPointB;
    }

protected:
    void CreateContextMenu();
    QMouseEvent CreateMouseEvent();
    void FindConnectedObjects(u32 InstanceID, QList<u32>& rIDList);

signals:
    void InputProcessed(const SRayIntersection& rkIntersect, QMouseEvent *pEvent);
    void ViewportClick(const SRayIntersection& rkIntersect, QMouseEvent *pEvent);
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
    void OnSelectConnected();
    void OnHideSelection();
    void OnHideUnselected();
    void OnHideNode();
    void OnHideType();
    void OnHideLayer();
    void OnUnhideAll();
    void OnContextMenuClose();
};

#endif // CSCENEVIEWPORT_H
