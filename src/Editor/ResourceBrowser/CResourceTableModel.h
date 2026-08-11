#ifndef CRESOURCETABLEMODEL
#define CRESOURCETABLEMODEL

#include <QAbstractTableModel>

class CResourceBrowser;
class CResourceEntry;
class CVirtualDirectory;
class TString;

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
    ~CResourceTableModel() override;

    // Interface
    int rowCount(const QModelIndex& = QModelIndex()) const override;
    int columnCount(const QModelIndex& = QModelIndex()) const override;
    QVariant data(const QModelIndex& rkIndex, int Role) const override;
    Qt::ItemFlags flags(const QModelIndex& rkIndex) const override;

    bool canDropMimeData(const QMimeData *pkData, Qt::DropAction Action, int Row, int Column, const QModelIndex& rkParent) const override;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;
    QMimeData* mimeData(const QModelIndexList& rkIndexes) const override;
    Qt::DropActions supportedDragActions() const override;
    Qt::DropActions supportedDropActions() const override;

    // Functionality
    QModelIndex GetIndexForEntry(const CResourceEntry *pEntry) const;
    QModelIndex GetIndexForDirectory(const CVirtualDirectory *pDir) const;
    CResourceEntry* IndexEntry(const QModelIndex& rkIndex) const;
    CVirtualDirectory* IndexDirectory(const QModelIndex& rkIndex) const;
    bool IsIndexDirectory(const QModelIndex& rkIndex) const;
    bool HasParentDirectoryEntry() const;
    void FillEntryList(CVirtualDirectory *pDir, bool AssetListMode);
    void DisplayEntryList(QList<CResourceEntry*> rkEntries, QString rkListDescription);
protected:
    void RecursiveAddDirectoryContents(CVirtualDirectory *pDir);
    int EntryListIndex(const CResourceEntry *pEntry) const;

public:
    // Accessors
    uint32_t NumDirectories() const          { return mDirectories.size(); }
    uint32_t NumResources() const            { return mEntries.size(); }
    CVirtualDirectory* CurrentDir() const    { return mpCurrentDir; }
    bool IsDisplayingAssetList() const       { return mIsAssetListMode; }
    bool IsDisplayingUserEntryList() const   { return mIsDisplayingUserEntryList; }
    const QString& ModelDescription() const  { return mModelDescription; }

public slots:
    void RefreshAllIndices();

private slots:
    void CheckAddResource(CResourceEntry *pEntry);
    void CheckRemoveResource(CResourceEntry *pEntry);
    void CheckAddDirectory(CVirtualDirectory *pDir);
    void CheckRemoveDirectory(CVirtualDirectory *pDir);
    void OnResourceMoved(CResourceEntry *pEntry, CVirtualDirectory *pOldDir, const TString& OldName);
    void OnDirectoryMoved(CVirtualDirectory *pDir, CVirtualDirectory *pOldDir, const TString& OldName);
};

#endif // CRESOURCELISTMODEL

