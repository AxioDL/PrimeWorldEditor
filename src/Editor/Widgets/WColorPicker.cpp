#include "WColorPicker.h"
#include <iostream>
#include <QPainter>
#include <QRectF>
#include <QMouseEvent>
#include <QColorDialog>

WColorPicker::WColorPicker(QWidget *parent) : QWidget(parent)
{
    mColor = Qt::transparent;
}

WColorPicker::~WColorPicker()
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

void WColorPicker::keyPressEvent(QKeyEvent *Event)
{
    if (Event->key() == Qt::Key_Return)
    {
        QColorDialog ColorPick;
        ColorPick.setOptions(QColorDialog::ShowAlphaChannel);
        ColorPick.setCurrentColor(mColor);
        int result = ColorPick.exec();

        if (result)
        {
            mColor = ColorPick.currentColor();
            emit colorChanged(mColor);
        }
    }
}

void WColorPicker::mousePressEvent(QMouseEvent *)
{
    setFocus();
}

void WColorPicker::mouseReleaseEvent(QMouseEvent *Event)
{
    if ((Event->x() < width()) && (Event->y() < height()))
    {
        QColorDialog ColorPick;
        ColorPick.setOptions(QColorDialog::ShowAlphaChannel);
        ColorPick.setCurrentColor(mColor);
        int result = ColorPick.exec();

        if (result)
        {
            mColor = ColorPick.currentColor();
            emit colorChanged(mColor);
        }
    }
}

QColor WColorPicker::getColor()
{
    return mColor;
}

void WColorPicker::setColor(QColor Color)
{
    if (mColor != Color)
    {
        mColor = Color;
        emit colorChanged(mColor);
        update();
    }
}
