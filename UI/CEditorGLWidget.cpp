#include <GL/glew.h>
#include <gtc/matrix_transform.hpp>
#include <gtx/transform.hpp>
#include <Core/CGraphics.h>
#include <Core/CRenderer.h>
#include <QTimer>
#include "CEditorGLWidget.h"
#include <iostream>
#include <Core/CGraphics.h>
#include <Resource/factory/CTextureDecoder.h>
#include <Common/CTimer.h>
#include <Common/CVector4f.h>
#include <QOpenGLContext>
#include <QPainter>
#include <QOpenGLPaintDevice>

QTimer CEditorGLWidget::sRefreshTimer;

CEditorGLWidget::CEditorGLWidget(QWidget *pParent) :
    QOpenGLWidget(pParent)
{
    setMouseTracking(true);
    mLastDrawTime = CTimer::GlobalTime();
    mKeysPressed = 0;
    mButtonsPressed = 0;
    mCursorState = Qt::ArrowCursor;
    mCursorVisible = true;
    mCamera.SetAspectRatio((float) width() / height());

    connect(&sRefreshTimer, SIGNAL(timeout()), this, SLOT(update()));

    if (!sRefreshTimer.isActive())
        sRefreshTimer.start(0);
}

CEditorGLWidget::~CEditorGLWidget()
{
}

void CEditorGLWidget::initializeGL()
{
    // Initialize CGraphics
    CGraphics::Initialize();

    // Setting various GL flags
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_PRIMITIVE_RESTART);
    glPrimitiveRestartIndex(0xFFFF);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_BLEND);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(1.f, 5.f);

    // Clear cached material
    CMaterial::KillCachedMaterial();
    CShader::KillCachedShader();

    // Initialize renderer
    emit ViewportResized(width(), height());
}

void CEditorGLWidget::paintGL()
{
    double DeltaTime = CTimer::GlobalTime() - mLastDrawTime;
    mLastDrawTime = CTimer::GlobalTime();

    // Camera movement is processed here in order to sync it with the paint event
    // This way movement happens exactly once per frame - no more, no less
    ProcessInput(DeltaTime);
    mCamera.LoadMatrices();

    // Pre-render signal allows for per-frame operations to be performed before the draw happens
    emit PreRender();

    // We emit a signal to indicate it's time to render the viewport instead of doing the rendering here.
    // This allows the editor GL widget class to be reused among multiple editors with different rendering needs.
    emit Render(mCamera);

    // Post-render signal allows for the frame to be completed with post-processing
    emit PostRender();
}

void CEditorGLWidget::resizeGL(int w, int h)
{
    mCamera.SetAspectRatio((float) w / h);
    glViewport(0, 0, w, h);
    emit ViewportResized(w, h);
}

void CEditorGLWidget::mouseMoveEvent(QMouseEvent *pEvent)
{
    if ((!IsMouseInputActive()) && (mButtonsPressed & eLeftButton))
        emit MouseDrag(pEvent);
}

void CEditorGLWidget::mousePressEvent(QMouseEvent *pEvent)
{
    setFocus();

    if (pEvent->button() == Qt::MidButton)   mButtonsPressed |= eMiddleButton;
    if (pEvent->button() == Qt::RightButton) mButtonsPressed |= eRightButton;

    if (IsMouseInputActive())
        SetCursorVisible(false);

    // Left click only activates if mouse input is inactive to prevent the user from
    // clicking on things and creating selection rectangles while the cursor is hidden
    else
    {
        if (pEvent->button() == Qt::LeftButton)
            mButtonsPressed |= eLeftButton;

        emit MouseClick(pEvent);
    }

    mLastMousePos = pEvent->globalPos();
}

void CEditorGLWidget::mouseReleaseEvent(QMouseEvent *pEvent)
{
    bool fromMouseInput = IsMouseInputActive();
    if (pEvent->button() == Qt::LeftButton)  mButtonsPressed &= ~eLeftButton;
    if (pEvent->button() == Qt::MidButton)   mButtonsPressed &= ~eMiddleButton;
    if (pEvent->button() == Qt::RightButton) mButtonsPressed &= ~eRightButton;

    // Make cursor visible if needed
    if (!IsMouseInputActive())
        SetCursorVisible(true);

    // Emit mouse release event if we didn't just exit mouse input (or regardless on left click)
    if (!fromMouseInput || (pEvent->button() == Qt::LeftButton))
        emit MouseRelease(pEvent);
}

void CEditorGLWidget::keyPressEvent(QKeyEvent *pEvent)
{
    switch (pEvent->key())
    {
    case Qt::Key_Q: mKeysPressed |= eQKey; break;
    case Qt::Key_W: mKeysPressed |= eWKey; break;
    case Qt::Key_E: mKeysPressed |= eEKey; break;
    case Qt::Key_A: mKeysPressed |= eAKey; break;
    case Qt::Key_S: mKeysPressed |= eSKey; break;
    case Qt::Key_D: mKeysPressed |= eDKey; break;
    case Qt::Key_Control: mKeysPressed |= eCtrlKey; break;
    }
}

void CEditorGLWidget::keyReleaseEvent(QKeyEvent *pEvent)
{
    switch (pEvent->key())
    {
    case Qt::Key_Q: mKeysPressed &= ~eQKey; break;
    case Qt::Key_W: mKeysPressed &= ~eWKey; break;
    case Qt::Key_E: mKeysPressed &= ~eEKey; break;
    case Qt::Key_A: mKeysPressed &= ~eAKey; break;
    case Qt::Key_S: mKeysPressed &= ~eSKey; break;
    case Qt::Key_D: mKeysPressed &= ~eDKey; break;
    case Qt::Key_Control: mKeysPressed &= ~eCtrlKey; break;
    }
}

void CEditorGLWidget::wheelEvent(QWheelEvent *pEvent)
{
    // Maybe track a "wheel delta" member variable and let CCamera decide what to do with it?
    mCamera.Zoom(pEvent->angleDelta().y() / 6000.f);
}

void CEditorGLWidget::focusOutEvent(QFocusEvent*)
{
    // When the widget loses focus, release all input.
    mButtonsPressed = 0;
    mKeysPressed = 0;
    SetCursorVisible(true);
}

void CEditorGLWidget::SetCursorState(const QCursor &Cursor)
{
    mCursorState = Cursor;

    if (IsCursorVisible())
        setCursor(Cursor);
}

void CEditorGLWidget::SetCursorVisible(bool visible)
{
    mCursorVisible = visible;

    if (visible)
        setCursor(mCursorState);
    else
        setCursor(Qt::BlankCursor);
}

bool CEditorGLWidget::IsCursorVisible()
{
    return mCursorVisible;
}

bool CEditorGLWidget::IsMouseInputActive()
{
    static const int skMoveButtons = eMiddleButton | eRightButton;
    return ((mButtonsPressed & skMoveButtons) != 0);
}

bool CEditorGLWidget::IsKeyboardInputActive()
{
    static const int skMoveKeys = eQKey | eWKey | eEKey | eAKey | eSKey | eDKey;
    return ((mKeysPressed & skMoveKeys) != 0);
}

CCamera& CEditorGLWidget::Camera()
{
    return mCamera;
}

CRay CEditorGLWidget::CastRay()
{
    CVector2f MouseCoords = MouseDeviceCoordinates();
    return mCamera.CastRay(MouseCoords);
}

CVector2f CEditorGLWidget::MouseDeviceCoordinates()
{
    QPoint MousePos = QCursor::pos();
    QPoint ThisPos = this->mapToGlobal(pos());
    MousePos -= ThisPos;

    CVector2f Device(
        (((2.f * MousePos.x()) / width()) - 1.f),
        (1.f - ((2.f * MousePos.y()) / height()))
    );
    return Device;
}


// ************ PRIVATE ************
void CEditorGLWidget::ProcessInput(double DeltaTime)
{
    if (IsMouseInputActive())
    {
        float XMovement = (QCursor::pos().x() - mLastMousePos.x()) * 0.01f;
        float YMovement = (QCursor::pos().y() - mLastMousePos.y()) * 0.01f;

        if ((XMovement != 0) || (YMovement != 0))
        {
            mCamera.ProcessMouseInput((EKeyInputs) mKeysPressed, (EMouseInputs) mButtonsPressed, XMovement, YMovement);
            QCursor::setPos(mLastMousePos);
        }
    }

    if (IsKeyboardInputActive())
        mCamera.ProcessKeyInput((EKeyInputs) mKeysPressed, DeltaTime);
}
