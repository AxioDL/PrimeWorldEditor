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

    bool eventFilter(QObject* pObj, QEvent* pEvent) override;

protected:
    void wheelEvent(QWheelEvent* pEvent) override;
};

#endif // WINTEGRALSPINBOX_H
