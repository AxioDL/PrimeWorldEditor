#include "WIntegralSpinBox.h"
#include <QWheelEvent>

WIntegralSpinBox::WIntegralSpinBox(QWidget *pParent) : QSpinBox(pParent)
{
}

WIntegralSpinBox::~WIntegralSpinBox()
{
}

void WIntegralSpinBox::wheelEvent(QWheelEvent *pEvent)
{
    if (!hasFocus()) pEvent->ignore();
    else QSpinBox::wheelEvent(pEvent);
}
