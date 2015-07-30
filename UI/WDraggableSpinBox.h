#ifndef WDRAGGABLESPINBOX_H
#define WDRAGGABLESPINBOX_H

#include <QDoubleSpinBox>

class WDraggableSpinBox : public QDoubleSpinBox
{
    Q_OBJECT
    bool mBeingDragged;
    bool mBeenDragged;
    double mDefaultValue;
    int mLastY;

public:
    explicit WDraggableSpinBox(QWidget *parent = 0);
    ~WDraggableSpinBox();
    void mousePressEvent(QMouseEvent *pEvent);
    void mouseReleaseEvent(QMouseEvent *pEvent);
    void mouseMoveEvent(QMouseEvent *pEvent);
    void wheelEvent(QWheelEvent *pEvent);
    bool eventFilter(QObject *pObj, QEvent *pEvent);
    void SetDefaultValue(double value);
};

#endif // WDRAGGABLESPINBOX_H
