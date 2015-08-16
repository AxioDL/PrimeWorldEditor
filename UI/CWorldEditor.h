#ifndef CWORLDEDITOR_H
#define CWORLDEDITOR_H

#include <QMainWindow>
#include <QList>

#include "CGizmo.h"
#include <Common/CRay.h>
#include <Common/CTimer.h>
#include <Common/EKeyInputs.h>
#include <Common/SRayIntersection.h>
#include <Core/CRenderer.h>
#include <Core/CSceneManager.h>
#include <Core/CToken.h>
#include <Resource/CGameArea.h>
#include <Resource/CWorld.h>

namespace Ui {
class CWorldEditor;
}

class CWorldEditor : public QMainWindow
{
    Q_OBJECT
    CRenderer *mpRenderer;
    CSceneManager *mpSceneManager;
    CGizmo mGizmo;
    CCamera mCamera;
    CGameArea *mpArea;
    CWorld *mpWorld;
    CToken mAreaToken;
    CToken mWorldToken;
    CTimer mFrameTimer;
    bool mDrawSky;
    bool mShowGizmo;

    CVector3f mHoverPoint;
    CSceneNode *mpHoverNode;
    std::list<CSceneNode*> mSelectedNodes;
    CAABox mSelectionAABox;

    CTimer mFPSTimer;
    int mFrameCount;

public:
    explicit CWorldEditor(QWidget *parent = 0);
    ~CWorldEditor();
    bool eventFilter(QObject *pObj, QEvent *pEvent);
    void SetArea(CWorld *pWorld, CGameArea *pArea);
    void ViewportRayCast(CRay Ray);
    CRenderer* Renderer();
    CSceneManager* Scene();
    CGameArea* ActiveArea();

    // Selection
    void SelectNode(CSceneNode *pNode);
    void DeselectNode(CSceneNode *pNode);
    void ClearSelection();

public slots:
    void ViewportPreRender();
    void ViewportRender(CCamera& Camera);
    void ViewportPostRender();
    void ViewportMouseDrag(QMouseEvent *pEvent);
    void ViewportMouseClick(QMouseEvent *pEvent);
    void SetViewportSize(int Width, int Height);

private:
    Ui::CWorldEditor *ui;
    void RecalculateSelectionBounds();
    void ResetHover();
    void UpdateCursor();

    // UI
    void OnSidebarResize();
    void UpdateSelectionUI();
    void UpdateStatusBar();

private slots:
    void OnCameraSpeedChange(double speed);
    void on_ActionDrawWorld_triggered();
    void on_ActionDrawCollision_triggered();
    void on_ActionDrawObjects_triggered();
    void on_ActionDrawLights_triggered();
    void on_ActionDrawSky_triggered();
    void on_ActionNoLighting_triggered();
    void on_ActionBasicLighting_triggered();
    void on_ActionWorldLighting_triggered();
    void on_ActionNoBloom_triggered();
    void on_ActionBloomMaps_triggered();
    void on_ActionFakeBloom_triggered();
    void on_ActionBloom_triggered();
    void on_ActionZoomOnSelection_triggered();
    void on_ActionDisableBackfaceCull_triggered();
    void on_ActionDisableAlpha_triggered();
    void on_ActionEditLayers_triggered();
    void on_ActionSelectObjects_triggered();
    void on_ActionTranslate_triggered();
    void on_ActionRotate_triggered();
    void on_ActionScale_triggered();
    void on_ActionIncrementGizmo_triggered();
    void on_ActionDecrementGizmo_triggered();
};

#endif // CWORLDEDITOR_H
