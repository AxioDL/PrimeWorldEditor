#ifndef WINTEGRALSPINBOX_H
#define WINTEGRALSPINBOX_H

#include <QSpinBox>

// Simple subclass to disable focus stealing on wheel event
class WIntegralSpinBox : public QSpinBox
{
    Q_OBJECT
public:
    explicit WIntegralSpinBox(QWidget *pParent);
    ~WIntegralSpinBox();
    void wheelEvent(QWheelEvent *pEvent);
    bool eventFilter(QObject *pObj, QEvent *pEvent);
};

#endif // WINTEGRALSPINBOX_H
