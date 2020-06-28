#ifndef WCOLORPICKER_H
#define WCOLORPICKER_H

#include <QWidget>
#include <QColor>

class WColorPicker : public QWidget
{
    Q_OBJECT
    QColor mColor{Qt::transparent};
    QColor mOldColor;

public:
    explicit WColorPicker(QWidget* pParent = nullptr);

    void paintEvent(QPaintEvent*) override;
    void keyPressEvent(QKeyEvent* pEvent) override;
    void mousePressEvent(QMouseEvent*) override;
    void mouseReleaseEvent(QMouseEvent* pEvent) override;
    QColor Color() const;
    void SetColor(QColor Color);

signals:
    void ColorChanged(QColor NewColor);
    void ColorEditComplete(QColor NewColor);

private slots:
    void DialogColorChanged(QColor NewColor);
    void DialogRejected();
};

#endif // WCOLORPICKER_H
