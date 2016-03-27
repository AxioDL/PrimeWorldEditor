#ifndef WDRAGGABLESPINBOX_H
#define WDRAGGABLESPINBOX_H

#include <QDoubleSpinBox>

class WDraggableSpinBox : public QDoubleSpinBox
{
    Q_OBJECT
    bool mBeingDragged;
    bool mBeenDragged;
    double mDefaultValue;
    int mLastY;
    int mMinDecimals;
    bool mTrimTrailingZeroes;

public:
    explicit WDraggableSpinBox(QWidget *pParent = 0);
    ~WDraggableSpinBox();
    void mousePressEvent(QMouseEvent *pEvent);
    void mouseReleaseEvent(QMouseEvent *pEvent);
    void mouseMoveEvent(QMouseEvent *pEvent);
    void wheelEvent(QWheelEvent *pEvent);
    bool eventFilter(QObject *pObj, QEvent *pEvent);
    QString textFromValue(double Val) const;
    bool IsBeingDragged();
    void SetDefaultValue(double Value);
    void SetMinDecimals(int Dec);
    void TrimTrailingZeroes(bool Trim);
};

#endif // WDRAGGABLESPINBOX_H
