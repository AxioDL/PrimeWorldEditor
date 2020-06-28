#ifndef CSTRINGDELEGATE_H
#define CSTRINGDELEGATE_H

#include "Editor/CCustomDelegate.h"

/** Delegate for rendering string entries in the string editor list view */
class CStringDelegate : public CCustomDelegate
{
public:
    explicit CStringDelegate(QObject* pParent = nullptr);

    SDelegateFontInfo GetFontInfo(const QStyleOptionViewItem& rkOption) const override;
    QSize sizeHint(const QStyleOptionViewItem& kOption, const QModelIndex& kIndex) const override;
    void paint(QPainter* pPainter, const QStyleOptionViewItem& kOption, const QModelIndex& kIndex) const override;
};

#endif // CSTRINGDELEGATE_H
