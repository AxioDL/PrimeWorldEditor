#ifndef CCUSTOMDELEGATE_H
#define CCUSTOMDELEGATE_H

#include <QFont>
#include <QFontMetrics>
#include <QPen>
#include <QStyledItemDelegate>

/** Font parameters for rendering text */
struct SDelegateFontInfo
{
    QFont NameFont;
    QFont InfoFont;
    QFontMetrics NameFontMetrics;
    QFontMetrics InfoFontMetrics;
    QPen NamePen;
    QPen InfoPen;
    int Margin = 0;
    int Spacing = 0;

    SDelegateFontInfo()
        : NameFontMetrics(NameFont), InfoFontMetrics(InfoFont) {}
};

/** Common base class of custom item delegate implementations */
class CCustomDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit CCustomDelegate(QObject* pParent = nullptr)
        : QStyledItemDelegate(pParent)
    {}

    virtual SDelegateFontInfo GetFontInfo(const QStyleOptionViewItem& rkOption) const
    {
        SDelegateFontInfo Info;

        Info.NameFont = rkOption.font;
        Info.NameFont.setPointSize( rkOption.font.pointSize() + 1 );
        Info.NameFontMetrics = QFontMetrics(Info.NameFont);

        Info.InfoFont = rkOption.font;
        Info.InfoFont.setPointSize( rkOption.font.pointSize() - 1 );
        Info.InfoFontMetrics = QFontMetrics(Info.InfoFont);

        Info.NamePen = QPen(rkOption.palette.text(), 1.f);

        Info.InfoPen = QPen(rkOption.palette.text(), 1.f);
        Info.InfoPen.setColor( Info.InfoPen.color().darker(140) );

        Info.Margin = 3;
        Info.Spacing = 3;

        return Info;
    }
};

#endif // CCUSTOMDELEGATE_H
