#include "CResourceDelegate.h"
#include "CResourceProxyModel.h"
#include "CResourceTableModel.h"
#include <Common/Common.h>

#include <QPainter>

struct SResDelegateInfo
{
    QFont NameFont;
    QFont InfoFont;
    QFontMetrics NameFontMetrics;
    QFontMetrics InfoFontMetrics;
    QPen NamePen;
    QPen InfoPen;
    int Margin;
    int Spacing;

    SResDelegateInfo()
        : NameFontMetrics(NameFont), InfoFontMetrics(InfoFont) {}
};
SResDelegateInfo GetDelegateInfo(const QStyleOptionViewItem& rkOption)
{
    SResDelegateInfo Info;

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

QSize CResourceBrowserDelegate::sizeHint(const QStyleOptionViewItem& rkOption, const QModelIndex&) const
{
    // Get string info
    SResDelegateInfo Info = GetDelegateInfo(rkOption);

    // Calculate height
    int Height = (Info.Margin * 2) + Info.NameFontMetrics.height() + Info.Spacing + Info.InfoFontMetrics.height();
    return QSize(0, Height);
}

void CResourceBrowserDelegate::paint(QPainter *pPainter, const QStyleOptionViewItem& rkOption, const QModelIndex& rkIndex) const
{
    const CResourceProxyModel *pkProxy = qobject_cast<const CResourceProxyModel*>(rkIndex.model());
    ASSERT(pkProxy != nullptr);

    const CResourceTableModel *pkModel = qobject_cast<const CResourceTableModel*>(pkProxy->sourceModel());
    ASSERT(pkModel != nullptr);

    // Get resource entry
    QModelIndex SourceIndex = pkProxy->mapToSource(rkIndex);
    CResourceEntry *pEntry = pkModel->IndexEntry(SourceIndex);

    // Initialize
    SResDelegateInfo Info = GetDelegateInfo(rkOption);
    QRect InnerRect = rkOption.rect.adjusted(Info.Margin, Info.Margin, -Info.Margin, -Info.Margin);
    QPoint PainterPos = InnerRect.topLeft();

    // Draw icon
    QVariant IconVariant = rkIndex.model()->data(rkIndex, Qt::DecorationRole);

    if (IconVariant != QVariant::Invalid)
    {
        QIcon Icon = IconVariant.value<QIcon>();

        // Determine icon size. Ideally 24x24 if we have space, but downscale if we don't
        int IdealIconSize = 24;
        int IconSize = Math::Min(InnerRect.height(), IdealIconSize);

        // Adjust icon position so it's centered in the ideal rect
        QPoint IconPos = PainterPos;
        IconPos.rx() += (IdealIconSize - IconSize) / 2;
        IconPos.ry() += (InnerRect.height() - IconSize) / 2;

        // Paint the icon
        QRect IconRect(IconPos, QSize(IconSize, IconSize));
        Icon.paint(pPainter, IconRect);

        PainterPos.rx() += IdealIconSize + Info.Spacing + Info.Spacing;
    }

    // Calculate rects
    if (!pEntry)
        PainterPos.ry() += (InnerRect.height() - Info.NameFontMetrics.height()) / 2;

    int ResNameWidth = InnerRect.width() - (PainterPos.x() - InnerRect.left());
    QSize ResNameSize(ResNameWidth, Info.NameFontMetrics.height());
    QRect ResNameRect = QRect(PainterPos, ResNameSize);
    PainterPos.ry() += ResNameRect.height() + Info.Spacing;

    int ResInfoWidth = ResNameWidth;
    QSize ResInfoSize(ResInfoWidth, Info.InfoFontMetrics.height());
    QRect ResInfoRect = QRect(PainterPos, ResInfoSize);

    // Draw resource name
    QString ResName = pkModel->data(SourceIndex, Qt::DisplayRole).toString();
    QString ElidedName = Info.NameFontMetrics.elidedText(ResName, Qt::ElideRight, ResNameWidth);

    pPainter->setFont(Info.NameFont);
    pPainter->setPen(Info.NamePen);
    pPainter->drawText(ResNameRect, ElidedName);

    // Draw resource info string
    if (pEntry)
    {
        QString ResInfo = TO_QSTRING( pEntry->TypeInfo()->TypeName() );

        if (mDisplayAssetIDs)
            ResInfo.prepend( TO_QSTRING(pEntry->ID().ToString()) + " | " );

        QString ElidedResInfo = Info.InfoFontMetrics.elidedText(ResInfo, Qt::ElideRight, ResInfoWidth);

        pPainter->setFont(Info.InfoFont);
        pPainter->setPen(Info.InfoPen);
        pPainter->drawText(ResInfoRect, ElidedResInfo);
    }
}
