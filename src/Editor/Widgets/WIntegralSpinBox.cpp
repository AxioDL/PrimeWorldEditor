#include "WIntegralSpinBox.h"
#include <QLineEdit>
#include <QWheelEvent>

WIntegralSpinBox::WIntegralSpinBox(QWidget *pParent) : QSpinBox(pParent)
{
    lineEdit()->installEventFilter(this);
}

WIntegralSpinBox::~WIntegralSpinBox() = default;

void WIntegralSpinBox::wheelEvent(QWheelEvent *pEvent)
{
    if (!hasFocus()) pEvent->ignore();
    else QSpinBox::wheelEvent(pEvent);
}

bool WIntegralSpinBox::eventFilter(QObject* /*pObj*/, QEvent *pEvent)
{
    if (pEvent->type() == QEvent::MouseButtonPress)
        setFocus();

    return false;
}
