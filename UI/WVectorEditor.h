#ifndef WVECTOREDITOR_H
#define WVECTOREDITOR_H

#include <QWidget>
#include <QHBoxLayout>
#include "WDraggableSpinBox.h"
#include <Common/CVector3f.h>
#include <QGroupBox>
#include <QLabel>
#include <QFormLayout>

class WVectorEditor : public QWidget
{
    Q_OBJECT

    CVector3f mValue;
    WDraggableSpinBox *mpSpinBoxX;
    WDraggableSpinBox *mpSpinBoxY;
    WDraggableSpinBox *mpSpinBoxZ;
    QHBoxLayout *mpLayout;

    // new layout test
    QGroupBox *mpGroupBox;
    QLabel *mpLabelX;
    QLabel *mpLabelY;
    QLabel *mpLabelZ;
    QFormLayout *mpFormLayout;

public:
    explicit WVectorEditor(QWidget *pParent = 0);
    WVectorEditor(const CVector3f& Value, QWidget *pParent = 0);
    ~WVectorEditor();
    CVector3f Value();
    void SetValue(const CVector3f& Value);
    void SetText(const QString& Text);

public slots:
    void SetX(double x);
    void SetY(double y);
    void SetZ(double z);
};

#endif // WVECTOREDITOR_H
