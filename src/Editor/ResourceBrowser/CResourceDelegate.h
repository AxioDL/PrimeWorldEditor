#ifndef CRESOURCEBROWSERDELEGATE_H
#define CRESOURCEBROWSERDELEGATE_H

#include <QStyledItemDelegate>

class CResourceBrowserDelegate : public QStyledItemDelegate
{
    bool mDisplayAssetIDs;

public:
    CResourceBrowserDelegate(QObject *pParent = 0)
        : QStyledItemDelegate(pParent)
        , mDisplayAssetIDs(false)
    {}

    QSize sizeHint(const QStyleOptionViewItem& rkOption, const QModelIndex& rkIndex) const;
    void paint(QPainter *pPainter, const QStyleOptionViewItem& rkOption, const QModelIndex& rkIndex) const;

    inline void SetDisplayAssetIDs(bool Display)    { mDisplayAssetIDs = Display; }
};

#endif // CRESOURCEBROWSERDELEGATE_H
