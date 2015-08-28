#ifndef CWORLDEDITOR_H
#define CWORLDEDITOR_H

#include <QMainWindow>
#include <QList>
#include <QComboBox>

#include "CGizmo.h"
#include <Common/CRay.h>
#include <Common/CTimer.h>
#include <Common/EKeyInputs.h>
#include <Common/SRayIntersection.h>
#include <Common/ETransformSpace.h>
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
    ETransformSpace mTranslateSpace;
    ETransformSpace mRotateSpace;
    CCamera mCamera;
    CGameArea *mpArea;
    CWorld *mpWorld;
    CToken mAreaToken;
    CToken mWorldToken;
    CTimer mFrameTimer;
    bool mDrawSky;
    bool mShowGizmo;
    bool mGizmoHovering;
    bool mGizmoTransforming;
    bool mGizmoUIOutdated;

    CVector3f mHoverPoint;
    CSceneNode *mpHoverNode;
    std::list<CSceneNode*> mSelectedNodes;
    CAABox mSelectionAABox;

    QComboBox *mpTransformSpaceComboBox;

    CTimer mFPSTimer;
    int mFrameCount;

public:
    explicit CWorldEditor(QWidget *parent = 0);
    ~CWorldEditor();
    bool eventFilter(QObject *pObj, QEvent *pEvent);
    void SetArea(CWorld *pWorld, CGameArea *pArea);
    void ViewportRayCast();
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
    void ViewportMouseRelease(QMouseEvent *pEvent);
    void SetViewportSize(int Width, int Height);
    void SetTransformSpace(int space);

private:
    Ui::CWorldEditor *ui;
    void RecalculateSelectionBounds();
    void ResetHover();
    void UpdateCursor();

    // UI
    void OnSidebarResize();
    void UpdateSelectionUI();
    void UpdateStatusBar();
    void UpdateGizmoUI();

private slots:
    void OnCameraSpeedChange(double speed);
    void OnTransformSpinBoxModified(CVector3f value);
    void OnTransformSpinBoxEdited(CVector3f value);
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
