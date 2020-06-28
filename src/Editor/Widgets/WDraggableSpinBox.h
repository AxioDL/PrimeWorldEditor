#ifndef WDRAGGABLESPINBOX_H
#define WDRAGGABLESPINBOX_H

#include <QDoubleSpinBox>

class WDraggableSpinBox : public QDoubleSpinBox
{
    Q_OBJECT
    bool mBeingDragged = false;
    bool mBeenDragged = false;
    double mDefaultValue = 0.0;
    int mLastY = 0;
    int mMinDecimals = 1;
    bool mTrimTrailingZeroes = true;

public:
    explicit WDraggableSpinBox(QWidget *pParent = nullptr);
    ~WDraggableSpinBox() override;

    void mousePressEvent(QMouseEvent* pEvent) override;
    void mouseReleaseEvent(QMouseEvent* pEvent) override;
    void mouseMoveEvent(QMouseEvent* pEvent) override;
    void wheelEvent(QWheelEvent* pEvent) override;
    bool eventFilter(QObject* pObj, QEvent* pEvent) override;
    QString textFromValue(double Val) const override;
    bool IsBeingDragged() const;
    void SetDefaultValue(double Value);
    void SetMinDecimals(int Dec);
    void TrimTrailingZeroes(bool Trim);
};

#endif // WDRAGGABLESPINBOX_H
