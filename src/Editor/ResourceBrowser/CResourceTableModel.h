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

    CVirtualDirectory *mpCurrentDir = nullptr;
    QList<CVirtualDirectory*> mDirectories;
    QList<CResourceEntry*> mEntries;
    QString mModelDescription;
    bool mIsAssetListMode = false;
    bool mIsDisplayingUserEntryList = false;

public:
    explicit CResourceTableModel(CResourceBrowser *pBrowser, QObject *pParent = nullptr);

    // Interface
    int rowCount(const QModelIndex& /*rkParent*/) const override;
    int columnCount(const QModelIndex& /*rkParent*/) const override;
    QVariant data(const QModelIndex& rkIndex, int Role) const override;
    Qt::ItemFlags flags(const QModelIndex& rkIndex) const override;

    bool canDropMimeData(const QMimeData *pkData, Qt::DropAction Action, int Row, int Column, const QModelIndex& rkParent) const override;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;
    QMimeData* mimeData(const QModelIndexList& rkIndexes) const override;
    Qt::DropActions supportedDragActions() const override;
    Qt::DropActions supportedDropActions() const override;

    // Functionality
    QModelIndex GetIndexForEntry(CResourceEntry *pEntry) const;
    QModelIndex GetIndexForDirectory(CVirtualDirectory *pDir) const;
    CResourceEntry* IndexEntry(const QModelIndex& rkIndex) const;
    CVirtualDirectory* IndexDirectory(const QModelIndex& rkIndex) const;
    bool IsIndexDirectory(const QModelIndex& rkIndex) const;
    bool HasParentDirectoryEntry() const;
    void FillEntryList(CVirtualDirectory *pDir, bool AssetListMode);
    void DisplayEntryList(QList<CResourceEntry*>& rkEntries, const QString& rkListDescription);
protected:
    void RecursiveAddDirectoryContents(CVirtualDirectory *pDir);
    int EntryListIndex(CResourceEntry *pEntry);

public:
    // Accessors
    uint32 NumDirectories() const            { return mDirectories.size(); }
    uint32 NumResources() const              { return mEntries.size(); }
    CVirtualDirectory* CurrentDir() const    { return mpCurrentDir; }
    bool IsDisplayingAssetList() const       { return mIsAssetListMode; }
    bool IsDisplayingUserEntryList() const   { return mIsDisplayingUserEntryList; }
    QString ModelDescription() const         { return mModelDescription; }

public slots:
    void RefreshAllIndices();
    void CheckAddResource(CResourceEntry *pEntry);
    void CheckRemoveResource(CResourceEntry *pEntry);
    void CheckAddDirectory(CVirtualDirectory *pDir);
    void CheckRemoveDirectory(CVirtualDirectory *pDir);
    void OnResourceMoved(CResourceEntry *pEntry, CVirtualDirectory *pOldDir, TString OldName);
    void OnDirectoryMoved(CVirtualDirectory *pDir, CVirtualDirectory *pOldDir, TString OldName);
};

#endif // CRESOURCELISTMODEL

