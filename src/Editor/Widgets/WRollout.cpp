#include "WRollout.h"
#include <QApplication>
#include <QBrush>
#include <QPainter>
#include <QPen>
#include <QRect>
#include <QVBoxLayout>

WRollout::WRollout(QWidget *parent) : QWidget(parent)
{
    this->setContentsMargins(10, 20, 10, 10);
}

WRollout::~WRollout()
{
}

void WRollout::setCollapsed(bool collapsed)
{
    mCollapsed = collapsed;
}

bool WRollout::isCollapsed()
{
    return mCollapsed;
}

void WRollout::setName(const QString& name)
{
    mpTopButton->setText(name);
}

QString WRollout::getName()
{
    return mpTopButton->text();
}

void WRollout::paintEvent(QPaintEvent *)
{
    QPainter Painter(this);

    // Draw box
    QPen Pen;
    Pen.setColor(Qt::white);
    Painter.setPen(Pen);
    QBrush Brush;
    Brush.setColor(Qt::white);
    Painter.setBrush(Brush);

    int AreaBoxTop = (mCollapsed) ? 7 : 10;
    QRect Area(QPoint(0,AreaBoxTop), size() - QSize(Pen.width(), Pen.width() + AreaBoxTop));
    Painter.drawRoundedRect(Area, 5.f, 5.f);

    // Draw button
    QRect TopButton(QPoint(10,0), QSize(width() - 20, 21));
    QPalette Palette = qApp->palette();
    Painter.setBrush(Palette.color(QPalette::Text));

    Painter.drawRect(TopButton);
}

void WRollout::resizeEvent(QResizeEvent *)
{
}
