#include "CStringDelegate.h"
#include <QPainter>

/** Constants */
static constexpr int gkMargin = 3;
static constexpr int gkSpacing = 3;

CStringDelegate::CStringDelegate(QObject* pParent /*= 0*/)
    : CCustomDelegate(pParent)
{}

SDelegateFontInfo CStringDelegate::GetFontInfo(const QStyleOptionViewItem& rkOption) const
{
    SDelegateFontInfo Info = CCustomDelegate::GetFontInfo(rkOption);
    Info.NameFont.setPointSize( rkOption.font.pointSize() );
    Info.NameFontMetrics = QFontMetrics(Info.NameFont);
    Info.NamePen.setColor( Info.NamePen.color().darker(120.f) );
    Info.InfoFont.setPointSize( rkOption.font.pointSize() );
    Info.InfoFont.setItalic(true);
    Info.InfoFontMetrics = QFontMetrics(Info.InfoFont);
    Info.InfoPen = QPen(rkOption.palette.text(), 1.f);
    return Info;
}

QSize CStringDelegate::sizeHint(const QStyleOptionViewItem& kOption, const QModelIndex& kIndex) const
{
    QSize BaseSize = CCustomDelegate::sizeHint(kOption, kIndex);

    SDelegateFontInfo FontInfo = GetFontInfo(kOption);
    int Height = (gkMargin * 2) + gkSpacing + (FontInfo.NameFontMetrics.height() * 2);
    return QSize(BaseSize.rwidth(), Height);
}

void CStringDelegate::paint(QPainter* pPainter, const QStyleOptionViewItem& kOption, const QModelIndex& kIndex) const
{
    SDelegateFontInfo FontInfo = GetFontInfo(kOption);
    QString StringName = kIndex.model()->data(kIndex, Qt::DisplayRole).toString();
    QString StringText = kIndex.model()->data(kIndex, Qt::UserRole).toString();
    QRect InnerRect = kOption.rect.adjusted(gkMargin, gkMargin, -gkMargin, -gkMargin);

    // Create font for the string number
    QFont NumberFont = FontInfo.NameFont;
    NumberFont.setPixelSize(InnerRect.height() * 0.6);

    // Dumb algorithm to calculate the number of digits
    int TotalNumStrings = kIndex.model()->rowCount();
    int NumDigits = 1;

    while (TotalNumStrings >= 10)
    {
        TotalNumStrings /= 10;
        NumDigits++;
    }

    // Add a buffer of one extra digit
    NumDigits++;

    int NumberWidth = QFontMetrics(NumberFont).averageCharWidth() * NumDigits;
    QRect NumRect(InnerRect.left(), InnerRect.top(), NumberWidth, InnerRect.height());

    // Calculate rects
    int X = InnerRect.left() + NumberWidth + gkMargin;
    int Width = InnerRect.width() - (X + gkMargin);
    int NameHeight = FontInfo.NameFontMetrics.height();
    int TextHeight = FontInfo.InfoFontMetrics.height();
    int NameY = kOption.rect.top() + gkMargin;
    int TextY = NameY + NameHeight + gkSpacing;

    QRect NameRect(X, NameY, Width, NameHeight);
    QRect TextRect(X, TextY, Width, TextHeight);

    // Elide name/text
    StringName = FontInfo.NameFontMetrics.elidedText(StringName, Qt::ElideRight, Width);
    StringText = FontInfo.InfoFontMetrics.elidedText(StringText, Qt::ElideRight, Width);

    // Draw selection rect
    if (kOption.state & QStyle::State_Selected)
    {
        pPainter->fillRect( kOption.rect, kOption.palette.highlight() );
    }

    // Draw number
    pPainter->setFont(NumberFont);
    pPainter->setPen(FontInfo.NamePen);
    pPainter->drawText(NumRect, Qt::AlignCenter, QString::number(kIndex.row() + 1));

    // Draw string info
    pPainter->setFont(FontInfo.NameFont);
    pPainter->setPen(FontInfo.NamePen);
    pPainter->drawText(NameRect, Qt::AlignLeft, StringName);

    pPainter->setFont(FontInfo.InfoFont);
    pPainter->setPen(FontInfo.InfoPen);
    pPainter->drawText(TextRect, Qt::AlignLeft, StringText);
}
