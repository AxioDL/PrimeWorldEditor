#include <iostream>

#include <QFileDialog>
#include <QMessageBox>

#include "CWorldEditorWindow.h"
#include <Resource/CTexture.h>
#include <Core/CResCache.h>
#include <Core/CSceneManager.h>
#include <FileIO/FileIO.h>
#include <Common/TString.h>
#include <Core/CGraphics.h>
#include <gtc/matrix_transform.hpp>
#include "CBasicViewport.h"

CWorldEditorWindow::CWorldEditorWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::CWorldEditorWindow)
{
    ui->setupUi(this);

    mpRenderer = new CRenderer();
    mpRenderer->ToggleGrid(false);
    mpActiveWorld = nullptr;
    mpActiveArea = nullptr;
    mRendererInitialized = false;
    mpSceneManager = new CSceneManager();
    mCamera.Snap(CVector3f(0, 3, 1));
    mCameraMode = eFreeCamera;
    mViewportKeysPressed = 0;
    mShouldDrawSky = true;

    /*connect(ui->CentralGLWidget, SIGNAL(ViewportResized(int,int)), this, SLOT(SetViewportSize(int,int)));
    connect(ui->CentralGLWidget, SIGNAL(PaintViewport(double)), this, SLOT(PaintViewport(double)));
    connect(ui->CentralGLWidget, SIGNAL(MouseClicked(QMouseEvent*)), this, SLOT(OnViewportRayCast(QMouseEvent*)));
    connect(ui->CentralGLWidget, SIGNAL(MouseMoved(QMouseEvent*, float, float)), this, SLOT(OnViewportMouseMove(QMouseEvent*, float, float)));
    connect(ui->CentralGLWidget, SIGNAL(KeyPressed(QKeyEvent*)), this, SLOT(OnViewportKeyPress(QKeyEvent*)));
    connect(ui->CentralGLWidget, SIGNAL(KeyReleased(QKeyEvent*)), this, SLOT(OnViewportKeyRelease(QKeyEvent*)));
    connect(ui->CentralGLWidget, SIGNAL(WheelScroll(int)), this, SLOT(OnViewportWheelScroll(int)));*/
}

CWorldEditorWindow::~CWorldEditorWindow()
{
    delete ui;
    delete mpRenderer;
    delete mpSceneManager;
}

void CWorldEditorWindow::InitializeWorld(CWorld *pWorld, CGameArea *pArea)
{
    mpSceneManager->SetActiveWorld(pWorld);
    mpSceneManager->SetActiveArea(pArea);
    mpRenderer->SetClearColor(CColor::skWhite);

    // Snap camera to location of area
    CTransform4f AreaTransform = pArea->GetTransform();
    CVector3f AreaPosition(AreaTransform[0][3], AreaTransform[1][3], AreaTransform[2][3]);
    mCamera.Snap(AreaPosition);

    // Set bloom based on world version
    if (pWorld != mpActiveWorld)
    {
        if (pWorld->Version() != eCorruption)
        {
            ui->menuBloom->setEnabled(false);
            on_actionDisableBloom_triggered();
        }

        else
        {
            ui->menuBloom->setEnabled(true);
            on_actionEnableBloom_triggered();
        }
    }

    mpActiveWorld = pWorld;
    mpActiveArea = pArea;
}

// ************ PUBLIC SLOTS ************
void CWorldEditorWindow::PaintViewport(double DeltaTime)
{
    if (!mRendererInitialized)
    {
        mpRenderer->Init();
        mRendererInitialized = true;
    }

    mCamera.ProcessKeyInput((EKeyInputs) mViewportKeysPressed, DeltaTime);
    mCamera.LoadMatrices();
    mpRenderer->BeginFrame();
    mpSceneManager->AddSceneToRenderer(mpRenderer, mCamera);


    if (mShouldDrawSky)
    {
        CModel *pSky = mpSceneManager->GetActiveSkybox();
        if (pSky) mpRenderer->RenderSky(pSky, mCamera);
    }

    mpRenderer->RenderBuckets(mCamera);
    mpRenderer->EndFrame();
}

void CWorldEditorWindow::SetViewportSize(int Width, int Height)
{
    mViewportAspectRatio = (float) Width / (float) Height;
    mpRenderer->SetViewportSize(Width, Height);
}

void CWorldEditorWindow::OnViewportMouseMove(QMouseEvent *pEvent, float XMovement, float YMovement)
{
    int KeyInputs = 0;
    if (pEvent->modifiers() & Qt::ControlModifier) KeyInputs |= eCtrlKey;
    if (pEvent->modifiers() & Qt::AltModifier)     KeyInputs |= eAltKey;

    int MouseInputs = 0;
    if (pEvent->buttons() & Qt::LeftButton)   MouseInputs |= eLeftButton;
    if (pEvent->buttons() & Qt::MiddleButton) MouseInputs |= eMiddleButton;
    if (pEvent->buttons() & Qt::RightButton)  MouseInputs |= eRightButton;

    mCamera.ProcessMouseInput((EKeyInputs) KeyInputs, (EMouseInputs) MouseInputs, XMovement, YMovement);
}

void CWorldEditorWindow::OnViewportKeyPress(QKeyEvent *pEvent)
{
    switch (pEvent->key())
    {
    case Qt::Key_Q: mViewportKeysPressed |= eQKey; break;
    case Qt::Key_W: mViewportKeysPressed |= eWKey; break;
    case Qt::Key_E: mViewportKeysPressed |= eEKey; break;
    case Qt::Key_A: mViewportKeysPressed |= eAKey; break;
    case Qt::Key_S: mViewportKeysPressed |= eSKey; break;
    case Qt::Key_D: mViewportKeysPressed |= eDKey; break;
    }
}

void CWorldEditorWindow::OnViewportKeyRelease(QKeyEvent *pEvent)
{
    switch (pEvent->key())
    {
    case Qt::Key_Q: mViewportKeysPressed &= ~eQKey; break;
    case Qt::Key_W: mViewportKeysPressed &= ~eWKey; break;
    case Qt::Key_E: mViewportKeysPressed &= ~eEKey; break;
    case Qt::Key_A: mViewportKeysPressed &= ~eAKey; break;
    case Qt::Key_S: mViewportKeysPressed &= ~eSKey; break;
    case Qt::Key_D: mViewportKeysPressed &= ~eDKey; break;
    }
}

void CWorldEditorWindow::OnViewportWheelScroll(int ScrollAmount)
{
    mCamera.Zoom(ScrollAmount / 6000.f);
}

void CWorldEditorWindow::OnViewportRayCast(QMouseEvent *pEvent)
{
    // todo: ray cast
}

// ************ PRIVATE SLOTS ************
void CWorldEditorWindow::LoadScriptableLayerUI()
{

}

void CWorldEditorWindow::on_actionExit_triggered()
{
    close();
}

void CWorldEditorWindow::on_actionBackface_culling_triggered()
{
    mpRenderer->ToggleBackfaceCull(ui->actionBackface_culling->isChecked());
}

void CWorldEditorWindow::on_actionWorld_triggered()
{
    mpRenderer->ToggleWorld(ui->actionWorld->isChecked());
}

void CWorldEditorWindow::on_actionCollision_triggered()
{
    mpRenderer->ToggleWorldCollision(ui->actionCollision->isChecked());
}

void CWorldEditorWindow::on_actionObjects_triggered()
{
    mpRenderer->ToggleObjects(ui->actionObjects->isChecked());
}

void CWorldEditorWindow::setupInstanceViewLayers()
{
/*    if (qApp->scene.MREAArray.empty()) return;

    mrea_GL *m = qApp->scene.MREAArray[0];
    if (!m->isSCLYRead()) return;

    u32 layer_count = m->getLayerCount();
    for (u32 l = 0; l < layer_count; l++) {
        QTreeWidgetItem* layer = new QTreeWidgetItem;
        layer->setText(0, "Layer " + QString::number(l));
        ui->InstanceViewTreeWidget->addTopLevelItem(layer);

        u32 object_count = m->getObjectCount(l);
        for (u32 o = 0; o < object_count; o++) {

            PrimeObject object = m->getObject(l, o);
            TString name = object.getStringProperty("Name");
            if (name.IsEmpty()) name = "[no name]";

            QTreeWidgetItem* obj = new QTreeWidgetItem;
            obj->setText(0, QString::fromStdString(name));
            obj->setText(1, QString::fromStdString(qApp->scene.getObjectName(object.type)));
            obj->setToolTip(0, obj->text(0));
            obj->setToolTip(1, obj->text(1));

            layer->addChild(obj);
            //layer->set
        }
    }*/
}

void CWorldEditorWindow::clearInstanceView()
{
    //ui->InstanceViewTreeWidget->clear();
}

void CWorldEditorWindow::on_actionMaterial_Animations_triggered()
{
    mpRenderer->ToggleUVAnimation(ui->actionMaterial_Animations->isChecked());
}

void CWorldEditorWindow::on_actionLights_triggered()
{
    mpRenderer->ToggleLights(ui->actionLights->isChecked());
}

void CWorldEditorWindow::on_actionLightingNone_triggered()
{
    CGraphics::sLightMode = CGraphics::NoLighting;
    ui->actionLightingNone->setChecked(true);
    ui->actionLightingBasic->setChecked(false);
    ui->actionLightingWorld->setChecked(false);
}

void CWorldEditorWindow::on_actionLightingBasic_triggered()
{
    CGraphics::sLightMode = CGraphics::BasicLighting;
    ui->actionLightingNone->setChecked(false);
    ui->actionLightingBasic->setChecked(true);
    ui->actionLightingWorld->setChecked(false);
}

void CWorldEditorWindow::on_actionLightingWorld_triggered()
{
    CGraphics::sLightMode = CGraphics::WorldLighting;
    ui->actionLightingNone->setChecked(false);
    ui->actionLightingBasic->setChecked(false);
    ui->actionLightingWorld->setChecked(true);
}

void CWorldEditorWindow::on_actionSky_triggered()
{
    mShouldDrawSky = ui->actionSky->isChecked();
}

void CWorldEditorWindow::on_actionOccluder_meshes_triggered()
{
    mpRenderer->ToggleOccluders(ui->actionOccluder_meshes->isChecked());
}

void CWorldEditorWindow::closeEvent(QCloseEvent *)
{
    emit Closed();
}

void CWorldEditorWindow::on_actionDisableBloom_triggered()
{
    mpRenderer->SetBloom(CRenderer::eNoBloom);
    ui->actionEnableBloom->setChecked(false);
    ui->actionDisableBloom->setChecked(true);
    ui->actionShowBloomMaps->setChecked(false);
}

void CWorldEditorWindow::on_actionEnableBloom_triggered()
{
    mpRenderer->SetBloom(CRenderer::eBloom);
    ui->actionDisableBloom->setChecked(false);
    ui->actionEnableBloom->setChecked(true);
    ui->actionShowBloomMaps->setChecked(false);
}

void CWorldEditorWindow::on_actionShowBloomMaps_triggered()
{
    mpRenderer->SetBloom(CRenderer::eBloomMaps);
    ui->actionDisableBloom->setChecked(false);
    ui->actionEnableBloom->setChecked(false);
    ui->actionShowBloomMaps->setChecked(true);
}
