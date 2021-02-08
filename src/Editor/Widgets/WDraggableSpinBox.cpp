#include "WDraggableSpinBox.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QLineEdit>
#include <QMouseEvent>
#include <QScreen>

WDraggableSpinBox::WDraggableSpinBox(QWidget *parent)
    : QDoubleSpinBox(parent)
{
    setMinimum(-1000000.0);
    setMaximum(1000000.0);
    setDecimals(6);
    lineEdit()->installEventFilter(this);
}

WDraggableSpinBox::~WDraggableSpinBox() = default;

void WDraggableSpinBox::mousePressEvent(QMouseEvent *pEvent)
{
    if (pEvent->button() == Qt::LeftButton)
    {
        mBeingDragged = true;
        mBeenDragged = false;
        mLastY = QCursor::pos().y();
    }
}

void WDraggableSpinBox::mouseReleaseEvent(QMouseEvent *pEvent)
{
    mBeingDragged = false;
    setCursor(Qt::ArrowCursor);

    if (pEvent->button() == Qt::LeftButton)
    {
        if (!mBeenDragged)
        {
            if (pEvent->y() <= height() / 2)
                stepUp();
            else
                stepDown();
        }
    }

    else if (pEvent->button() == Qt::RightButton)
    {
        setValue(mDefaultValue);
    }

    emit editingFinished();
}

void WDraggableSpinBox::mouseMoveEvent(QMouseEvent*)
{
    if (mBeingDragged)
    {
        QPoint CursorPos = QCursor::pos();

        // Update value
        double DragAmount = (singleStep() / 10.0) * (mLastY - CursorPos.y());
        setValue(value() + DragAmount);

        // Wrap cursor
        int ScreenHeight = QApplication::primaryScreen()->geometry().height();

        if (CursorPos.y() == ScreenHeight - 1)
            QCursor::setPos(CursorPos.x(), 1);
        if (CursorPos.y() == 0)
            QCursor::setPos(CursorPos.x(), ScreenHeight - 2);

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

QString WDraggableSpinBox::textFromValue(double Val) const
{
    QString Str = QString::number(Val, 'f', decimals());
    int DecIndex = Str.indexOf('.');
    int NumDecs;

    if (DecIndex == -1)
        NumDecs = 0;
    else
        NumDecs = Str.size() - DecIndex - 1;

    if (NumDecs < mMinDecimals)
    {
        int Size = Str.size() + mMinDecimals + 1;
        Str.reserve(Size);

        Str += '.';

        for (int iDec = 0; iDec < mMinDecimals; iDec++)
            Str += '0';
    }

    else if ((NumDecs > mMinDecimals) && mTrimTrailingZeroes)
    {
        while (NumDecs > mMinDecimals)
        {
            if (Str.endsWith('0'))
            {
                Str.chop(1);
                NumDecs--;
            }

            else if (Str.endsWith('.'))
            {
                Str.chop(1);
                break;
            }

            else break;
        }
    }

    return Str;
}

bool WDraggableSpinBox::IsBeingDragged() const
{
    return mBeingDragged;
}

void WDraggableSpinBox::SetDefaultValue(double Value)
{
    mDefaultValue = Value;
}

void WDraggableSpinBox::SetMinDecimals(int Dec)
{
    mMinDecimals = Dec;
}

void WDraggableSpinBox::TrimTrailingZeroes(bool Trim)
{
    mTrimTrailingZeroes = Trim;
}
