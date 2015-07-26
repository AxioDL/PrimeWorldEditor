#ifndef CEDITORGLWIDGET_H
#define CEDITORGLWIDGET_H

#include <QTimer>
#include <gl/glew.h>
#include <QOpenGLWidget>
#include <QMouseEvent>
#include <Common/CVector2f.h>
#include <Common/CVector2i.h>
#include <Core/CSceneManager.h>
#include <Core/CRenderer.h>
#include <Resource/CFont.h>
#include <Common/CRay.h>

class CEditorGLWidget : public QOpenGLWidget
{
    Q_OBJECT

    static QTimer sRefreshTimer;
    CCamera mCamera;
    QPoint mLastMousePos;
    double mLastDrawTime;
    QPoint mLeftClickPoint;
    int mButtonsPressed; // int container for EMouseInputs flags
    int mKeysPressed;    // int container for EKeyInputs flags
    QCursor mCursorState;
    bool mCursorVisible;

public:
    explicit CEditorGLWidget(QWidget *pParent = 0);
    ~CEditorGLWidget();
    void initializeGL();
    void paintGL();
    void resizeGL(int w, int h);
    void mouseMoveEvent(QMouseEvent *pEvent);
    void mousePressEvent(QMouseEvent *pEvent);
    void mouseReleaseEvent(QMouseEvent *pEvent);
    void keyPressEvent(QKeyEvent *pEvent);
    void keyReleaseEvent(QKeyEvent *pEvent);
    void wheelEvent(QWheelEvent *pEvent);
    void focusOutEvent(QFocusEvent *pEvent);
    void SetCursorState(const QCursor& Cursor);
    void SetCursorVisible(bool visible);
    bool IsCursorVisible();
    bool IsMouseInputActive();
    bool IsKeyboardInputActive();
    CCamera& Camera();
    CRay CastRay();
    CVector2f MouseDeviceCoordinates();

signals:
    void ViewportResized(int w, int h);
    void PreRender();
    void Render(CCamera& Camera);
    void PostRender();
    void MouseClick(QMouseEvent *pEvent);
    void MouseDrag(QMouseEvent *pEvent);

private:
    void ProcessInput(double DeltaTime);
};

#endif
