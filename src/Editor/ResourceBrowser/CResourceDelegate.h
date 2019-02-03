#ifndef CRESOURCEBROWSERDELEGATE_H
#define CRESOURCEBROWSERDELEGATE_H

#include "Editor/CCustomDelegate.h"

class CResourceBrowserDelegate : public CCustomDelegate
{
public:
    static const int skIconSize = 32;

private:
    bool mDisplayAssetIDs;

public:
    CResourceBrowserDelegate(QObject *pParent = 0)
        : CCustomDelegate(pParent)
        , mDisplayAssetIDs(false)
    {}

    QSize sizeHint(const QStyleOptionViewItem& rkOption, const QModelIndex& rkIndex) const;
    void paint(QPainter *pPainter, const QStyleOptionViewItem& rkOption, const QModelIndex& rkIndex) const;

    QWidget* createEditor(QWidget *pParent, const QStyleOptionViewItem& rkOption, const QModelIndex& rkIndex) const;
    void setEditorData(QWidget *pEditor, const QModelIndex& rkIndex) const;
    void setModelData(QWidget *pEditor, QAbstractItemModel *pModel, const QModelIndex& rkIndex) const;
    void updateEditorGeometry(QWidget *pEditor, const QStyleOptionViewItem& rkOption, const QModelIndex& rkIndex) const;

    inline void SetDisplayAssetIDs(bool Display)    { mDisplayAssetIDs = Display; }

protected:
    class CResourceEntry* GetIndexEntry(const QModelIndex& rkIndex) const;
    class CVirtualDirectory* GetIndexDirectory(const QModelIndex& rkIndex) const;
};

#endif // CRESOURCEBROWSERDELEGATE_H
