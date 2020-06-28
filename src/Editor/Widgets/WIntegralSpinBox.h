#ifndef WINTEGRALSPINBOX_H
#define WINTEGRALSPINBOX_H

#include <QSpinBox>

// Simple subclass to disable focus stealing on wheel event
class WIntegralSpinBox : public QSpinBox
{
    Q_OBJECT
public:
    explicit WIntegralSpinBox(QWidget *pParent);
    ~WIntegralSpinBox() override;

    void wheelEvent(QWheelEvent* pEvent) override;
    bool eventFilter(QObject* pObj, QEvent* pEvent) override;
};

#endif // WINTEGRALSPINBOX_H
