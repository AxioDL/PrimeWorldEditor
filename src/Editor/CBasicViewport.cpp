#include "CBasicViewport.h"
#include <Common/Math/MathUtil.h>
#include <Core/Render/CDrawUtil.h>
#include <Core/Render/CGraphics.h>
#include <Editor/MacOSExtras.h>

#include <QCursor>

CBasicViewport::CBasicViewport(QWidget *pParent)
    : QOpenGLWidget(pParent)
{
    setMouseTracking(true);
    mCamera.SetAspectRatio((float) width() / height());
    mViewInfo.ShowFlags = EShowFlag::All;
    mViewInfo.pCamera = &mCamera;
    mViewInfo.GameMode = false;
}

CBasicViewport::~CBasicViewport() = default;

void CBasicViewport::initializeGL()
{
    // Initialize CGraphics
    CGraphics::Initialize();

    // Setting various GL flags
    glEnable(GL_BLEND);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_PRIMITIVE_RESTART);
    glPrimitiveRestartIndex(0xFFFF);
    glPolygonOffset(1.f, 5.f);
    glDepthFunc(GL_LEQUAL);

    // Clear cached material
    CMaterial::KillCachedMaterial();
    CShader::KillCachedShader();

    // Initialize size
    OnResize();
}

void CBasicViewport::paintGL()
{
    // Prep render
    float scale = devicePixelRatioF();
    glViewport(0, 0, (int)((float)width() * scale), (int)((float)height() * scale));
    glLineWidth(1.f);
    glEnable(GL_DEPTH_TEST);
    mViewInfo.ViewFrustum = mCamera.FrustumPlanes();
    CGraphics::sMVPBlock.ProjectionMatrix = mCamera.ProjectionMatrix();

    // Actual rendering is intended to be handled by subclassing CBasicViewport and
    // reimplementing Paint().
    Paint();

    // Finally, draw XYZ axes in the corner
    if (!mViewInfo.GameMode)
        DrawAxes();
}

void CBasicViewport::resizeGL(int Width, int Height)
{
    mCamera.SetAspectRatio((float) Width / Height);
    float scale = devicePixelRatioF();
    glViewport(0, 0, (int)((float)Width * scale), (int)((float)Height * scale));
    OnResize();
}

void CBasicViewport::mousePressEvent(QMouseEvent *pEvent)
{
    setFocus();

    if (pEvent->button() == Qt::MiddleButton) mButtonsPressed |= EMouseInput::MiddleButton;
    if (pEvent->button() == Qt::RightButton) mButtonsPressed |= EMouseInput::RightButton;

    if (IsMouseInputActive())
    {
#if __APPLE__
        // This will zero out the drag accumulators
        gpMouseDragCocoaEventFilter->claimX();
        gpMouseDragCocoaEventFilter->claimY();
#endif
        SetCursorVisible(false);
        mMouseMoved = false;
        mMoveTimer.Restart();
    }

    // Left click only activates if mouse input is inactive to prevent the user from
    // clicking on things and creating selection rectangles while the cursor is hidden
    else
    {
        if (pEvent->button() == Qt::LeftButton)
            mButtonsPressed |= EMouseInput::LeftButton;

        OnMouseClick(pEvent);
    }

    mLastMousePos = pEvent->globalPos();
}

void CBasicViewport::mouseReleaseEvent(QMouseEvent *pEvent)
{
    bool fromMouseInput = IsMouseInputActive();
    if (pEvent->button() == Qt::LeftButton)   mButtonsPressed &= ~EMouseInput::LeftButton;
    if (pEvent->button() == Qt::MiddleButton) mButtonsPressed &= ~EMouseInput::MiddleButton;
    if (pEvent->button() == Qt::RightButton)  mButtonsPressed &= ~EMouseInput::RightButton;

    // Make cursor visible if needed
    if (!IsMouseInputActive())
        SetCursorVisible(true);

    // Run mouse release if we didn't just exit mouse input (or regardless on left click)
    if (!fromMouseInput || (pEvent->button() == Qt::LeftButton))
        OnMouseRelease(pEvent);

    // Send context menu event to subclass if needed
    if ((pEvent->button() == Qt::RightButton) && (mMoveTimer.Time() <= 0.3) && !mMouseMoved)
    {
        QContextMenuEvent Event(QContextMenuEvent::Mouse, QCursor::pos());
        this->ContextMenu(&Event);
    }
 }

void CBasicViewport::mouseMoveEvent(QMouseEvent* /*pEvent*/)
{
    // todo: draggable selection rectangle
}

void CBasicViewport::wheelEvent(QWheelEvent *pEvent)
{
    // Maybe track a "wheel delta" member variable and let CCamera decide what to do with it?
    mCamera.Zoom(pEvent->angleDelta().y() / 240.f);
}

void CBasicViewport::keyPressEvent(QKeyEvent *pEvent)
{
    switch (pEvent->key())
    {
    case Qt::Key_Q: mKeysPressed |= EKeyInput::Q; break;
    case Qt::Key_W: mKeysPressed |= EKeyInput::W; break;
    case Qt::Key_E: mKeysPressed |= EKeyInput::E; break;
    case Qt::Key_A: mKeysPressed |= EKeyInput::A; break;
    case Qt::Key_S: mKeysPressed |= EKeyInput::S; break;
    case Qt::Key_D: mKeysPressed |= EKeyInput::D; break;
    case Qt::Key_Control: mKeysPressed |= EKeyInput::Ctrl; break;
    case Qt::Key_Shift: mKeysPressed |= EKeyInput::Shift; break;
    case Qt::Key_Alt: mKeysPressed |= EKeyInput::Alt; break;
    }
}

void CBasicViewport::keyReleaseEvent(QKeyEvent *pEvent)
{
    switch (pEvent->key())
    {
    case Qt::Key_Q: mKeysPressed &= ~EKeyInput::Q; break;
    case Qt::Key_W: mKeysPressed &= ~EKeyInput::W; break;
    case Qt::Key_E: mKeysPressed &= ~EKeyInput::E; break;
    case Qt::Key_A: mKeysPressed &= ~EKeyInput::A; break;
    case Qt::Key_S: mKeysPressed &= ~EKeyInput::S; break;
    case Qt::Key_D: mKeysPressed &= ~EKeyInput::D; break;
    case Qt::Key_Control: mKeysPressed &= ~EKeyInput::Ctrl; break;
    case Qt::Key_Shift: mKeysPressed &= ~EKeyInput::Shift; break;
    case Qt::Key_Alt: mKeysPressed &= ~EKeyInput::Alt; break;
    }
}

void CBasicViewport::focusOutEvent(QFocusEvent*)
{
    // When the widget loses focus, release all input.
    mButtonsPressed = 0;
    mKeysPressed = 0;
    SetCursorVisible(true);
}

void CBasicViewport::contextMenuEvent(QContextMenuEvent *pEvent)
{
    pEvent->ignore();
}

void CBasicViewport::SetShowFlag(EShowFlag Flag, bool Visible)
{
    if (Visible)
        mViewInfo.ShowFlags |= Flag;
    else
        mViewInfo.ShowFlags &= ~Flag;
}

void CBasicViewport::SetGameMode(bool Enabled)
{
    mViewInfo.GameMode = Enabled;
}

void CBasicViewport::SetCursorState(const QCursor& rkCursor)
{
    mCursorState = rkCursor;

    if (IsCursorVisible())
        setCursor(rkCursor);
}

void CBasicViewport::SetCursorVisible(bool Visible)
{
    mCursorVisible = Visible;

    if (Visible)
        setCursor(mCursorState);
    else
        setCursor(Qt::BlankCursor);
}

bool CBasicViewport::IsCursorVisible() const
{
    return mCursorVisible;
}

bool CBasicViewport::IsMouseInputActive() const
{
    static constexpr FMouseInputs skMoveButtons = EMouseInput::MiddleButton | EMouseInput::RightButton;
    return ((mButtonsPressed & skMoveButtons) != 0);
}

bool CBasicViewport::IsKeyboardInputActive() const
{
    static constexpr FKeyInputs skMoveKeys = EKeyInput::Q | EKeyInput::W | EKeyInput::E |
                                             EKeyInput::A | EKeyInput::S | EKeyInput::D;
    return ((mKeysPressed & skMoveKeys) != 0);
}

CCamera& CBasicViewport::Camera()
{
    return mCamera;
}

const CCamera& CBasicViewport::Camera() const
{
    return mCamera;
}

CRay CBasicViewport::CastRay() const
{
    CVector2f MouseCoords = MouseDeviceCoordinates();
    return mCamera.CastRay(MouseCoords);
}

CVector2f CBasicViewport::MouseDeviceCoordinates() const
{
    QPoint MousePos = mapFromGlobal(QCursor::pos());

    CVector2f Device(
        (((2.f * MousePos.x()) / width()) - 1.f),
        (1.f - ((2.f * MousePos.y()) / height()))
    );
    return Device;
}

double CBasicViewport::LastRenderDuration()
{
    return mFrameTimer.Time();
}

// ************ PUBLIC SLOTS ************
void CBasicViewport::ProcessInput()
{
    // Process camera input
    double DeltaTime = CTimer::GlobalTime() - mLastDrawTime;
    mLastDrawTime = CTimer::GlobalTime();

    if (IsMouseInputActive())
    {
        float XMovement, YMovement;
#ifdef __APPLE__
        // QCursor::setPos only works on macOS when the user permits PWE
        // to control the computer via Universal Access.
        // As an alternative to relying on the delta of a warped mouse,
        // use the accumulated delta directly reported by AppKit.
        if (!AXIsProcessTrusted())
        {
            XMovement = gpMouseDragCocoaEventFilter->claimX() * 0.01f;
            YMovement = gpMouseDragCocoaEventFilter->claimY() * 0.01f;
        }
        else
#endif
        {
            XMovement = (QCursor::pos().x() - mLastMousePos.x()) * 0.01f;
            YMovement = (QCursor::pos().y() - mLastMousePos.y()) * 0.01f;
        }

        if ((XMovement != 0) || (YMovement != 0))
        {
            mCamera.ProcessMouseInput((FKeyInputs) mKeysPressed, (FMouseInputs) mButtonsPressed, XMovement, YMovement);
            QCursor::setPos(mLastMousePos);
            mMouseMoved = true;
        }
    }

    if (IsKeyboardInputActive())
        if ((mKeysPressed & EKeyInput::Ctrl) == 0)
            mCamera.ProcessKeyInput((FKeyInputs) mKeysPressed, DeltaTime);

    // Update view info
    const CMatrix4f& rkView = mCamera.ViewMatrix();
    mViewInfo.RotationOnlyViewMatrix = CMatrix4f(rkView[0][0], rkView[0][1], rkView[0][2], 0.f,
                                                 rkView[1][0], rkView[1][1], rkView[1][2], 0.f,
                                                 rkView[2][0], rkView[2][1], rkView[2][2], 0.f,
                                                 rkView[3][0], rkView[3][1], rkView[3][2], 1.f);

    mViewInfo.ViewFrustum = mCamera.FrustumPlanes();

    // Check user input
    CheckUserInput();
}

void CBasicViewport::Render()
{
    mFrameTimer.Start();
    update();
    mFrameTimer.Stop();
}

// ************ PRIVATE ************
void CBasicViewport::DrawAxes()
{
    // Draw 64x64 axes in lower-left corner with 8px margins
    glBlendFunc(GL_ONE, GL_ZERO);
    glViewport(8, 8, 64, 64);
    glEnable(GL_DEPTH_TEST);
    glClear(GL_DEPTH_BUFFER_BIT);
    glDepthRange(0.f, 1.f);

    CGraphics::sMVPBlock.ModelMatrix = CTransform4f::TranslationMatrix(mCamera.Direction() * 5);
    CGraphics::sMVPBlock.ViewMatrix = mViewInfo.RotationOnlyViewMatrix;
    CGraphics::sMVPBlock.ProjectionMatrix = Math::OrthographicMatrix(-1.f, 1.f, -1.f, 1.f, 0.1f, 100.f);
    CGraphics::UpdateMVPBlock();

    glLineWidth(1.f);
    CDrawUtil::DrawLine(CVector3f::Zero(), CVector3f(1,0,0), CColor::Red());   // X
    CDrawUtil::DrawLine(CVector3f::Zero(), CVector3f(0,1,0), CColor::Green()); // Y
    CDrawUtil::DrawLine(CVector3f::Zero(), CVector3f(0,0,1), CColor::Blue());  // Z
}
