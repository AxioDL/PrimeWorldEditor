#ifndef WTEXTUREGLWIDGET_H
#define WTEXTUREGLWIDGET_H

#include <Math/CTransform4f.h>
#include <Math/CVector2f.h>
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
    u32 mContextID;
    bool mInitialized;

public:
    explicit WTextureGLWidget(QWidget *parent = 0, CTexture *pTex = 0);
    ~WTextureGLWidget();
    void initializeGL();
    void paintGL();
    void resizeGL(int w, int h);
    void SetTexture(CTexture *pTex);

private:
    void CalcTexTransform();
    void CalcCheckerCoords();
};

#endif // WTEXTUREGLWIDGET_H
