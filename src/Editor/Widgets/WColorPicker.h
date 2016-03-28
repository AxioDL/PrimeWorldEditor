#ifndef WCOLORPICKER_H
#define WCOLORPICKER_H

#include <QWidget>
#include <QColor>

class WColorPicker : public QWidget
{
    Q_OBJECT
    QColor mColor;
    QColor mOldColor;

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
    void ColorEditComplete(QColor NewColor);

private slots:
    void DialogColorChanged(QColor NewColor);
    void DialogRejected();
};

#endif // WCOLORPICKER_H
