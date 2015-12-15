#ifndef CSCENEVIEWPORT_H
#define CSCENEVIEWPORT_H

#include "CBasicViewport.h"
#include "INodeEditor.h"

class CSceneViewport : public CBasicViewport
{
    Q_OBJECT

    INodeEditor *mpEditor;
    CSceneManager *mpScene;
    CRenderer *mpRenderer;

    // Render settings
    bool mDrawSky;

    // Scene interaction
    bool mGizmoHovering;
    bool mGizmoTransforming;
    CSceneNode *mpHoverNode;
    CVector3f mHoverPoint;

public:
    CSceneViewport(QWidget *pParent = 0);
    ~CSceneViewport();
    void SetScene(INodeEditor *pEditor, CSceneManager *pScene);
    void SetSkyEnabled(bool b);
    CRenderer* Renderer();
    CSceneNode* HoverNode();
    CVector3f HoverPoint();
    void CheckGizmoInput(const CRay& ray);
    void SceneRayCast(const CRay& ray);
    void ResetHover();
    bool IsHoveringGizmo();

    void keyPressEvent(QKeyEvent* pEvent);
    void keyReleaseEvent(QKeyEvent* pEvent);

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
};

#endif // CSCENEVIEWPORT_H
