#ifndef CEDITORGLWIDGET_H
#define CEDITORGLWIDGET_H

#include <Common/CTimer.h>
#include <Common/Math/CRay.h>
#include <Common/Math/CVector2i.h>
#include <Common/Math/CVector2f.h>
#include <Core/Render/CRenderer.h>
#include <Core/Render/SViewInfo.h>

#include <QOpenGLWidget>
#include <QMouseEvent>
#include <QPoint>
#include <QTimer>

class CBasicViewport : public QOpenGLWidget
{
    Q_OBJECT

protected:
    // Render
    CCamera mCamera;
    CTimer mFrameTimer;
    double mLastDrawTime = CTimer::GlobalTime();
    SViewInfo mViewInfo;

    // Cursor settings
    QCursor mCursorState{Qt::ArrowCursor};
    bool mCursorVisible = true;

    // Input
    QPoint mLastMousePos;
    bool mMouseMoved = false;
    CTimer mMoveTimer;
    FMouseInputs mButtonsPressed{0};
    FKeyInputs mKeysPressed{0};

public:
    explicit CBasicViewport(QWidget *pParent = nullptr);
    ~CBasicViewport() override;
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int Width, int Height) override;
    void mousePressEvent(QMouseEvent *pEvent) override;
    void mouseReleaseEvent(QMouseEvent *pEvent) override;
    void mouseMoveEvent(QMouseEvent *pEvent) override;
    void wheelEvent(QWheelEvent *pEvent) override;
    void keyPressEvent(QKeyEvent *pEvent) override;
    void keyReleaseEvent(QKeyEvent *pEvent) override;
    void focusOutEvent(QFocusEvent *pEvent) override;
    void contextMenuEvent(QContextMenuEvent *pEvent) override;

    void SetShowFlag(EShowFlag Flag, bool Visible);
    void SetGameMode(bool Enabled);
    void SetCursorState(const QCursor& rkCursor);
    void SetCursorVisible(bool Visible);
    bool IsCursorVisible() const;
    bool IsMouseInputActive() const;
    bool IsKeyboardInputActive() const;
    CCamera& Camera();
    const CCamera& Camera() const;
    CRay CastRay() const;
    CVector2f MouseDeviceCoordinates() const;
    double LastRenderDuration();

    SCollisionRenderSettings& CollisionRenderSettings()  { return mViewInfo.CollisionSettings; }
    const SCollisionRenderSettings& CollisionRenderSettings() const { return mViewInfo.CollisionSettings; }
public slots:
    void ProcessInput();
    void Render();

protected slots:
    virtual void CheckUserInput() {}
    virtual void Paint() {}
    virtual void ContextMenu(QContextMenuEvent* /*pEvent*/) {}
    virtual void OnResize() {}
    virtual void OnMouseClick(QMouseEvent* /*pEvent*/) {}
    virtual void OnMouseRelease(QMouseEvent* /*pEvent*/) {}

private:
    void DrawAxes();
};

#endif
