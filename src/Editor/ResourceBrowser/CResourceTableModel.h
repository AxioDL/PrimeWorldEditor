#ifndef CRESOURCETABLEMODEL
#define CRESOURCETABLEMODEL

#include "Editor/UICommon.h"
#include <Core/GameProject/CResourceEntry.h>
#include <Core/GameProject/CResourceIterator.h>
#include <Core/GameProject/CResourceStore.h>
#include <Core/Resource/CResource.h>
#include <QAbstractTableModel>
#include <QIcon>

class CResourceTableModel : public QAbstractTableModel
{
    Q_OBJECT

    CVirtualDirectory *mpCurrentDir;
    QList<CVirtualDirectory*> mDirectories;
    QList<CResourceEntry*> mEntries;
    bool mIsAssetListMode;

public:
    CResourceTableModel(CResourceBrowser *pBrowser, QObject *pParent = 0);

    // Interface
    int rowCount(const QModelIndex& /*rkParent*/) const;
    int columnCount(const QModelIndex& /*rkParent*/) const;
    QVariant data(const QModelIndex& rkIndex, int Role) const;
    Qt::ItemFlags flags(const QModelIndex& rkIndex) const;

    bool canDropMimeData(const QMimeData *pkData, Qt::DropAction Action, int Row, int Column, const QModelIndex& rkParent) const;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent);
    QMimeData* mimeData(const QModelIndexList& rkIndexes) const;
    Qt::DropActions supportedDragActions() const;
    Qt::DropActions supportedDropActions() const;

    // Functionality
    QModelIndex GetIndexForEntry(CResourceEntry *pEntry) const;
    QModelIndex GetIndexForDirectory(CVirtualDirectory *pDir) const;
    CResourceEntry* IndexEntry(const QModelIndex& rkIndex) const;
    CVirtualDirectory* IndexDirectory(const QModelIndex& rkIndex) const;
    bool IsIndexDirectory(const QModelIndex& rkIndex) const;
    void FillEntryList(CVirtualDirectory *pDir, bool AssetListMode);
protected:
    void RecursiveAddDirectoryContents(CVirtualDirectory *pDir);
    int EntryListIndex(CResourceEntry *pEntry);

public:
    // Accessors
    inline u32 NumDirectories() const   { return mDirectories.size(); }
    inline u32 NumResources() const     { return mEntries.size(); }

public slots:
    void CheckAddDirectory(CVirtualDirectory *pDir);
    void CheckRemoveDirectory(CVirtualDirectory *pDir);
    void OnResourceMoved(CResourceEntry *pEntry, CVirtualDirectory *pOldDir, TString OldName);
    void OnDirectoryMoved(CVirtualDirectory *pDir, CVirtualDirectory *pOldDir, TString OldName);
};

#endif // CRESOURCELISTMODEL

