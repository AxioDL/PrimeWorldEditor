#include "WDraggableSpinBox.h"
#include <QMouseEvent>
#include <QDesktopWidget>

WDraggableSpinBox::WDraggableSpinBox(QWidget *parent) : QDoubleSpinBox(parent)
{
    mBeingDragged = false;
    mDefaultValue = value();
    setMinimum(-1000000.0);
    setMaximum(1000000.0);
    setDecimals(4);
}

WDraggableSpinBox::~WDraggableSpinBox()
{
}

void WDraggableSpinBox::mousePressEvent(QMouseEvent *Event)
{
    mBeingDragged = true;
    mBeenDragged = false;
    mInitialY = Event->y();
    mInitialValue = value();
}

void WDraggableSpinBox::mouseReleaseEvent(QMouseEvent *Event)
{
    mBeingDragged = false;

    if (Event->button() == Qt::LeftButton)
    {
        if (!mBeenDragged)
        {
            if (Event->y() <= height() / 2)
                stepUp();
            else
                stepDown();
        }
    }

    else if (Event->button() == Qt::RightButton)
    {
        setValue(mDefaultValue);
    }
}

void WDraggableSpinBox::mouseMoveEvent(QMouseEvent *Event)
{
    if (mBeingDragged)
    {
        double DragAmount = (singleStep() / 10.0) * (mInitialY - Event->y());
        setValue(mInitialValue + DragAmount);
        if (DragAmount != 0) mBeenDragged = true;
    }
}
