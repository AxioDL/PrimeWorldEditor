#ifndef WDRAGGABLESPINBOX_H
#define WDRAGGABLESPINBOX_H

#include <QDoubleSpinBox>

class WDraggableSpinBox : public QDoubleSpinBox
{
    Q_OBJECT
    bool mBeingDragged;
    bool mBeenDragged;
    double mInitialValue;
    double mDefaultValue;
    int mInitialY;

public:
    explicit WDraggableSpinBox(QWidget *parent = 0);
    ~WDraggableSpinBox();
    void mousePressEvent(QMouseEvent *Event);
    void mouseReleaseEvent(QMouseEvent *Event);
    void mouseMoveEvent(QMouseEvent *Event);
};

#endif // WDRAGGABLESPINBOX_H
