#ifndef WVECTOREDITOR_H
#define WVECTOREDITOR_H

#include "WDraggableSpinBox.h"
#include <Common/Math/CVector3f.h>

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
    explicit WVectorEditor(QWidget *pParent = nullptr);
    explicit WVectorEditor(const CVector3f& rkValue, QWidget *pParent = nullptr);
    ~WVectorEditor() override;

    CVector3f Value() const;
    void SetOrientation(Qt::Orientation Orientation);
    void SetValue(const CVector3f& rkValue);
    void SetDefaultValue(double Value);
    void SetSingleStep(double Step);
    void SetLabelsHidden(bool Hidden);
    bool IsBeingDragged() const;
    bool IsBeingEdited() const;

public slots:
    void SetX(double X);
    void SetY(double Y);
    void SetZ(double Z);

signals:
    void ValueChanged(const CVector3f& rkValue);
    void EditingDone(const CVector3f& rkValue);

private:
    void SetupUI();

private slots:
    void OnSpinBoxEditingDone();
};

#endif // WVECTOREDITOR_H
