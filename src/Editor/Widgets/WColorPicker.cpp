#include "WColorPicker.h"
#include <QPainter>
#include <QRectF>
#include <QMouseEvent>
#include <QColorDialog>

WColorPicker::WColorPicker(QWidget *parent)
    : QWidget(parent)
{
}

void WColorPicker::paintEvent(QPaintEvent *)
{
    QRect Area(QPoint(2,2), size() - QSize(5,5)); // Subtraction makes room for the stroke
    QColor FillColor = mColor;
    FillColor.setAlpha(255);

    QBrush Fill(FillColor);
    QPen Outline(Qt::black, 1);

    QPainter Painter(this);
    Painter.setBrush(Fill);
    Painter.setPen(Outline);
    Painter.drawRect(Area);

    if (hasFocus())
    {
        QRect DottedLine(QPoint(0,0), size() - QSize(1,1));
        Fill.setColor(Qt::transparent);
        Outline.setStyle(Qt::DotLine);

        Painter.setBrush(Fill);
        Painter.setPen(Outline);
        Painter.drawRect(DottedLine);
    }
}

void WColorPicker::keyPressEvent(QKeyEvent *pEvent)
{
    if (pEvent->key() == Qt::Key_Return)
    {
        QColorDialog ColorPick;
        ColorPick.setOptions(QColorDialog::ShowAlphaChannel);
        ColorPick.setCurrentColor(mColor);
        int result = ColorPick.exec();

        if (result)
        {
            mColor = ColorPick.currentColor();
            emit ColorChanged(mColor);
        }
    }
}

void WColorPicker::mousePressEvent(QMouseEvent *)
{
    setFocus();
}

void WColorPicker::mouseReleaseEvent(QMouseEvent *pEvent)
{
    if ((pEvent->x() < width()) && (pEvent->y() < height()))
    {
        mOldColor = mColor;

        QColorDialog ColorPick(this);
        ColorPick.setOptions(QColorDialog::ShowAlphaChannel);
        ColorPick.setCurrentColor(mColor);
        connect(&ColorPick, &QColorDialog::currentColorChanged, this, &WColorPicker::DialogColorChanged);
        connect(&ColorPick, &QColorDialog::rejected, this, &WColorPicker::DialogRejected);
        int Result = ColorPick.exec();

        if (Result)
        {
            mColor = ColorPick.currentColor();
            emit ColorEditComplete(mColor);
        }
    }
}

QColor WColorPicker::Color() const
{
    return mColor;
}

void WColorPicker::SetColor(QColor Color)
{
    if (mColor != Color)
    {
        mColor = Color;
        emit ColorEditComplete(mColor);
        update();
    }
}

void WColorPicker::DialogColorChanged(QColor NewColor)
{
    mColor = NewColor;
    emit ColorChanged(mColor);
    update();
}

void WColorPicker::DialogRejected()
{
    SetColor(mOldColor);
}
