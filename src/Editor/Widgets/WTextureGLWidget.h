#ifndef WTEXTUREGLWIDGET_H
#define WTEXTUREGLWIDGET_H

#include <Common/Math/CTransform4f.h>
#include <Common/Math/CVector2f.h>
#include <Core/Resource/TResPtr.h>
#include <Core/Resource/CTexture.h>

#include <QOpenGLWidget>
#include <QTimer>
#include <GL/glew.h>

class WTextureGLWidget : public QOpenGLWidget
{
    Q_OBJECT

    float mAspectRatio;
    TResPtr<CTexture> mpTexture;
    float mTexAspectRatio;
    CTransform4f mTexTransform;
    CVector2f mCheckerCoords[4];
    uint32 mContextID;
    bool mInitialized;

public:
    explicit WTextureGLWidget(QWidget *pParent = 0, CTexture *pTex = 0);
    ~WTextureGLWidget();
    void initializeGL();
    void paintGL();
    void resizeGL(int Width, int Height);
    void SetTexture(CTexture *pTex);

private:
    void CalcTexTransform();
    void CalcCheckerCoords();
};

#endif // WTEXTUREGLWIDGET_H
