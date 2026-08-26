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

    const QColor& Color() const { return mColor; }
    void SetColor(const QColor& Color);

protected:
    void paintEvent(QPaintEvent*) override;
    void keyPressEvent(QKeyEvent* pEvent) override;
    void mousePressEvent(QMouseEvent*) override;
    void mouseReleaseEvent(QMouseEvent* pEvent) override;

signals:
    void ColorChanged(const QColor& NewColor);
    void ColorEditComplete(const QColor& NewColor);

private slots:
    void DialogColorChanged(const QColor& NewColor);
    void DialogRejected();
};

#endif // WCOLORPICKER_H
