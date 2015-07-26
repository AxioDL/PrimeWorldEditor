#ifndef CWORLDEDITORWINDOW_H
#define CWORLDEDITORWINDOW_H

#include <QMainWindow>
#include "ui_CWorldEditorWindow.h"

#include <Core/CRenderer.h>
#include <Core/CSceneManager.h>
#include <Core/CResCache.h>

namespace Ui {
class CWorldEditorWindow;
}

class CWorldEditorWindow : public QMainWindow
{
    Q_OBJECT
    CRenderer *mpRenderer;
    CSceneManager *mpSceneManager;
    CWorld *mpActiveWorld;
    CToken mWorldToken;
    CGameArea *mpActiveArea;
    CToken mAreaToken;

    bool mRendererInitialized;
    CCamera mCamera;
    ECameraMoveMode mCameraMode;
    float mViewportAspectRatio;
    int mViewportKeysPressed;
    bool mShouldDrawSky;

    void LoadScriptableLayerUI();

public:
    CWorldEditorWindow(QWidget *parent);
    ~CWorldEditorWindow();
    void InitializeWorld(CWorld *pWorld, CGameArea *pArea);

public slots:
    void PaintViewport(double DeltaTime);
    void SetViewportSize(int Width, int Height);
    void OnViewportMouseMove(QMouseEvent *pEvent, float XMovement, float YMovement);
    void OnViewportRayCast(QMouseEvent *pEvent);
    void OnViewportKeyPress(QKeyEvent *pEvent);
    void OnViewportKeyRelease(QKeyEvent *pEvent);
    void OnViewportWheelScroll(int ScrollAmount);

private slots:
    void on_actionExit_triggered();

    void on_actionBackface_culling_triggered();

    void on_actionWorld_triggered();

    void on_actionCollision_triggered();

    void on_actionObjects_triggered();

    void on_actionMaterial_Animations_triggered();

    void on_actionLights_triggered();

    void on_actionLightingNone_triggered();

    void on_actionLightingBasic_triggered();

    void on_actionLightingWorld_triggered();

    void on_actionSky_triggered();

    void on_actionOccluder_meshes_triggered();

    void on_actionDisableBloom_triggered();

    void on_actionEnableBloom_triggered();

    void on_actionShowBloomMaps_triggered();

signals:
    void Closed();

private:
    Ui::CWorldEditorWindow *ui;
    void setupInstanceViewLayers();
    void clearInstanceView();
    void closeEvent(QCloseEvent *);
};

#endif // CWORLDEDITORWINDOW_H
