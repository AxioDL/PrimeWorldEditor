#include "WDraggableSpinBox.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QLineEdit>
#include <QMouseEvent>

WDraggableSpinBox::WDraggableSpinBox(QWidget *parent) : QDoubleSpinBox(parent)
{
    mBeingDragged = false;
    mDefaultValue = 0;
    mMinDecimals = 1;
    mTrimTrailingZeroes = true;
    setMinimum(-1000000.0);
    setMaximum(1000000.0);
    setDecimals(6);
    lineEdit()->installEventFilter(this);
}

WDraggableSpinBox::~WDraggableSpinBox()
{
}

void WDraggableSpinBox::mousePressEvent(QMouseEvent *pEvent)
{
    if (pEvent->button() == Qt::LeftButton)
    {
        mBeingDragged = true;
        mBeenDragged = false;
        mLastY = QCursor::pos().y();
    }
}

void WDraggableSpinBox::mouseReleaseEvent(QMouseEvent *Event)
{
    mBeingDragged = false;
    setCursor(Qt::ArrowCursor);

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

    emit editingFinished();
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

void WDraggableSpinBox::wheelEvent(QWheelEvent *pEvent)
{
    if (!hasFocus()) pEvent->ignore();
    else QDoubleSpinBox::wheelEvent(pEvent);
}

bool WDraggableSpinBox::eventFilter(QObject *, QEvent *pEvent)
{
    if (pEvent->type() == QEvent::MouseButtonPress)
        setFocus();

    return false;
}

QString WDraggableSpinBox::textFromValue(double val) const
{
    QString str = QString::number(val, 'f', decimals());
    int decIndex = str.indexOf('.');
    int numDecs;

    if (decIndex == -1)
        numDecs = 0;
    else
        numDecs = str.size() - decIndex - 1;

    if (numDecs < mMinDecimals)
    {
        int size = str.size() + mMinDecimals + 1;
        str.reserve(size);

        str += '.';

        for (int iDec = 0; iDec < mMinDecimals; iDec++)
            str += '0';
    }

    else if ((numDecs > mMinDecimals) && mTrimTrailingZeroes)
    {
        while (numDecs > mMinDecimals)
        {
            if (str.endsWith('0')) {
                str.chop(1);
                numDecs--;
            }

            else if (str.endsWith('.')) {
                str.chop(1);
                break;
            }

            else break;
        }
    }

    return str;
}

bool WDraggableSpinBox::IsBeingDragged()
{
    return mBeingDragged;
}

void WDraggableSpinBox::SetDefaultValue(double value)
{
    mDefaultValue = value;
}

void WDraggableSpinBox::SetMinDecimals(int dec)
{
    mMinDecimals = dec;
}

void WDraggableSpinBox::TrimTrailingZeroes(bool trim)
{
    mTrimTrailingZeroes = trim;
}
