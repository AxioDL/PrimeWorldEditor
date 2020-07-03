#ifndef CRESOURCEBROWSERDELEGATE_H
#define CRESOURCEBROWSERDELEGATE_H

#include "Editor/CCustomDelegate.h"

class CResourceBrowserDelegate : public CCustomDelegate
{
public:
    static constexpr int skIconSize = 32;

private:
    bool mDisplayAssetIDs = false;

public:
    explicit CResourceBrowserDelegate(QObject *pParent = nullptr)
        : CCustomDelegate(pParent)
    {}

    QSize sizeHint(const QStyleOptionViewItem& rkOption, const QModelIndex& rkIndex) const override;
    void paint(QPainter* pPainter, const QStyleOptionViewItem& rkOption, const QModelIndex& rkIndex) const override;

    QWidget* createEditor(QWidget* pParent, const QStyleOptionViewItem& rkOption, const QModelIndex& rkIndex) const override;
    void setEditorData(QWidget* pEditor, const QModelIndex& rkIndex) const override;
    void setModelData(QWidget* pEditor, QAbstractItemModel* pModel, const QModelIndex& rkIndex) const override;
    void updateEditorGeometry(QWidget* pEditor, const QStyleOptionViewItem& rkOption, const QModelIndex& rkIndex) const override;

    void SetDisplayAssetIDs(bool Display)    { mDisplayAssetIDs = Display; }

protected:
    class CResourceEntry* GetIndexEntry(const QModelIndex& rkIndex) const;
    class CVirtualDirectory* GetIndexDirectory(const QModelIndex& rkIndex) const;
};

#endif // CRESOURCEBROWSERDELEGATE_H
