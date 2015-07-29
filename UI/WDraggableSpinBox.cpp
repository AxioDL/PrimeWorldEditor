#include "WDraggableSpinBox.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QMouseEvent>

WDraggableSpinBox::WDraggableSpinBox(QWidget *parent) : QDoubleSpinBox(parent)
{
    mBeingDragged = false;
    mDefaultValue = value();
    setMinimum(-1000000.0);
    setMaximum(1000000.0);
}

WDraggableSpinBox::~WDraggableSpinBox()
{
}

void WDraggableSpinBox::mousePressEvent(QMouseEvent*)
{
    mBeingDragged = true;
    mBeenDragged = false;
    mLastY = QCursor::pos().y();
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

        else
        {
            setCursor(Qt::ArrowCursor);
        }
    }

    else if (Event->button() == Qt::RightButton)
    {
        setValue(mDefaultValue);
    }
}

void WDraggableSpinBox::mouseMoveEvent(QMouseEvent*)
{
    if (mBeingDragged)
    {
        QPoint cursorPos = QCursor::pos();

        // Update value
        double DragAmount = (singleStep() / 10.0) * (mLastY - cursorPos.y());
        setValue(value() + DragAmount);

        // Wrap cursor
        int screenHeight = QApplication::desktop()->screenGeometry().height();

        if (cursorPos.y() == screenHeight - 1)
            QCursor::setPos(cursorPos.x(), 1);
        if (cursorPos.y() == 0)
            QCursor::setPos(cursorPos.x(), screenHeight - 2);

        // Set cursor shape
        if (DragAmount != 0)
        {
            mBeenDragged = true;
            setCursor(Qt::SizeVerCursor);
        }

        // Update last Y
        mLastY = QCursor::pos().y();
    }
}
