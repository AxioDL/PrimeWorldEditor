#ifndef WCOLORPICKER_H
#define WCOLORPICKER_H

#include <QWidget>
#include <QColor>

class WColorPicker : public QWidget
{
    Q_OBJECT
    QColor mColor;

public:
    explicit WColorPicker(QWidget *pParent = 0);
    void paintEvent(QPaintEvent *);
    void keyPressEvent(QKeyEvent *pEvent);
    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *pEvent);
    QColor Color();
    void SetColor(QColor Color);

signals:
    void ColorChanged(QColor NewColor);

public slots:
};

#endif // WCOLORPICKER_H
