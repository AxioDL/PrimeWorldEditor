#ifndef WCOLORPICKER_H
#define WCOLORPICKER_H

#include <QWidget>
#include <QColor>

class WColorPicker : public QWidget
{
    Q_OBJECT
    QColor mColor;

public:
    explicit WColorPicker(QWidget *parent = 0);
    ~WColorPicker();
    void paintEvent(QPaintEvent *);
    void keyPressEvent(QKeyEvent *Event);
    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *Event);
    QColor getColor();
    void setColor(QColor Color);

signals:
    void colorChanged(QColor NewColor);

public slots:
};

#endif // WCOLORPICKER_H
