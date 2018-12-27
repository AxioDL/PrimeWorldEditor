#ifndef CSTRINGDELEGATE_H
#define CSTRINGDELEGATE_H

#include "Editor/CCustomDelegate.h"

/** Delegate for rendering string entries in the string editor list view */
class CStringDelegate : public CCustomDelegate
{
public:
    CStringDelegate(QObject* pParent = 0);

    SDelegateFontInfo GetFontInfo(const QStyleOptionViewItem& rkOption) const;
    QSize sizeHint(const QStyleOptionViewItem& kOption, const QModelIndex& kIndex) const;
    void paint(QPainter* pPainter, const QStyleOptionViewItem& kOption, const QModelIndex& kIndex) const;
};

#endif // CSTRINGDELEGATE_H
