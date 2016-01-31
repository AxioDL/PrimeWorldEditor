#ifndef WVECTOREDITOR_H
#define WVECTOREDITOR_H

#include "WDraggableSpinBox.h"
#include <Math/CVector3f.h>

#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QWidget>

class WVectorEditor : public QWidget
{
    Q_OBJECT

    CVector3f mValue;
    bool mEditing;

    Qt::Orientation mOrientation;
    QLayout *mpLayout;
    QHBoxLayout *mpXLayout;
    QHBoxLayout *mpYLayout;
    QHBoxLayout *mpZLayout;
    WDraggableSpinBox *mpSpinBoxX;
    WDraggableSpinBox *mpSpinBoxY;
    WDraggableSpinBox *mpSpinBoxZ;
    QLabel *mpLabelX;
    QLabel *mpLabelY;
    QLabel *mpLabelZ;

public:
    explicit WVectorEditor(QWidget *pParent = 0);
    WVectorEditor(const CVector3f& value, QWidget *pParent = 0);
    ~WVectorEditor();
    CVector3f Value();
    void SetOrientation(Qt::Orientation orientation);
    void SetValue(const CVector3f& value);
    void SetDefaultValue(double value);
    void SetSingleStep(double step);
    void SetLabelsHidden(bool hidden);
    bool IsBeingDragged();

public slots:
    void SetX(double x);
    void SetY(double y);
    void SetZ(double z);

signals:
    void ValueChanged(const CVector3f& value);
    void EditingDone(const CVector3f& value);

private:
    void SetupUI();

private slots:
    void OnSpinBoxEditingDone();
};

#endif // WVECTOREDITOR_H
