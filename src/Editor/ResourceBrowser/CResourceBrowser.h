#ifndef CRESOURCEBROWSER_H
#define CRESOURCEBROWSER_H

#include "CResourceDelegate.h"
#include "CResourceProxyModel.h"
#include "CResourceTableModel.h"
#include "CVirtualDirectoryModel.h"

#include <QCheckBox>
#include <QMenu>
#include <QTimer>
#include <QUndoStack>
#include <QVBoxLayout>

namespace Ui {
class CResourceBrowser;
}

class CResourceBrowser : public QWidget
{
    Q_OBJECT
    std::unique_ptr<Ui::CResourceBrowser> mpUI;
    CResourceEntry *mpSelectedEntry = nullptr;
    CResourceStore *mpStore = nullptr;
    CResourceTableModel *mpModel = nullptr;
    CResourceProxyModel *mpProxyModel = nullptr;
    CResourceBrowserDelegate *mpDelegate = nullptr;
    CVirtualDirectory *mpSelectedDir = nullptr;
    CVirtualDirectoryModel *mpDirectoryModel = nullptr;
    bool mEditorStore = false;
    bool mAssetListMode = false;
    bool mSearching = false;

    // Add Menu
    QMenu *mpAddMenu = nullptr;

    // Type Filter
    QWidget *mpFilterBoxesContainerWidget = nullptr;
    QVBoxLayout *mpFilterBoxesLayout = nullptr;
    QCheckBox *mpFilterAllBox = nullptr;
    QFont mFilterBoxFont;

    struct SResourceType
    {
        CResTypeInfo *pTypeInfo;
        QCheckBox *pFilterCheckBox;
    };
    QList<SResourceType> mTypeList;

    // Undo/Redo
    QUndoStack mUndoStack;
    QAction *mpUndoAction = nullptr;
    QAction *mpRedoAction = nullptr;
    QWidget *mpActionContainerWidget = nullptr;

    // Misc
    CResourceEntry *mpInspectedEntry = nullptr; // Entry being "inspected" (viewing dependencies/referencers, etc)

public:
    explicit CResourceBrowser(QWidget *pParent = nullptr);
    ~CResourceBrowser() override;

    void SetActiveDirectory(CVirtualDirectory *pDir);
    void SelectResource(CResourceEntry *pEntry, bool ClearFiltersIfNecessary = false);
    void SelectDirectory(CVirtualDirectory *pDir);
    void CreateFilterCheckboxes();
    void CreateAddMenu();

    void AddCreateAssetMenuActions(QMenu* pMenu);

    bool RenameResource(CResourceEntry *pEntry, const TString& rkNewName);
    bool RenameDirectory(CVirtualDirectory *pDir, const TString& rkNewName);
    bool MoveResources(const QList<CResourceEntry*>& rkResources, const QList<CVirtualDirectory*>& rkDirectories, CVirtualDirectory *pNewDir);

    CResourceEntry* CreateNewResource(EResourceType Type,
                                      TString Name = "",
                                      CVirtualDirectory* pDir = nullptr,
                                      CAssetID ID = CAssetID());

    // Interface
    bool eventFilter(QObject* pWatched, QEvent* pEvent) override;

    // Accessors
    CResourceStore* CurrentStore() const     { return mpStore; }
    CResourceEntry* SelectedEntry() const    { return mpSelectedEntry; }
    bool InAssetListMode() const             { return mAssetListMode || mSearching || mpModel->IsDisplayingUserEntryList(); }

    void SetInspectedEntry(CResourceEntry *pEntry)   { mpInspectedEntry = pEntry; }

public slots:
    void RefreshResources();
    void RefreshDirectories();
    void UpdateDescriptionLabel();
    void SetResourceTreeView();
    void SetResourceListView();
    void OnClearButtonPressed();
    void OnSortModeChanged(int Index);
    void OnCreateAssetAction();
    bool CreateDirectory();
    bool Delete(QVector<CResourceEntry*> Resources, QVector<CVirtualDirectory*> Directories);
    void OnSearchStringChanged(const QString& SearchString);
    void OnDirectorySelectionChanged(const QModelIndex& rkNewIndex);
    void OnDoubleClickTable(QModelIndex Index);
    void OnResourceSelectionChanged(const QModelIndex& rkNewIndex);
    void FindAssetByID();
    void SetAssetIDDisplayEnabled(bool Enable);

    void UpdateStore();
    void SetProjectStore();
    void SetEditorStore();
    void ImportPackageContentsList();
    void GenerateAssetNames();
    void ImportAssetNameMap();
    void ExportAssetNames();
    void RebuildResourceDB();

    void ClearFilters();
    void ResetSearch();
    void ResetTypeFilter();
    void OnFilterTypeBoxTicked(bool Checked);
    void UpdateFilter();

    void UpdateUndoActionStates();
    void Undo();
    void Redo();

signals:
    void SelectedResourceChanged(CResourceEntry *pNewRes);

    void ResourceAboutToBeMoved(CResourceEntry *pRes, QString NewPath);
    void ResourceMoved(CResourceEntry *pRes, CVirtualDirectory *pOldDir, TString OldName);

    void ResourceAboutToBeCreated(CVirtualDirectory* pInDir);
    void ResourceCreated(CResourceEntry *pRes);

    void ResourceAboutToBeDeleted(CResourceEntry *pRes);
    void ResourceDeleted();

    void DirectoryAboutToBeMoved(CVirtualDirectory *pDir, QString NewPath);
    void DirectoryMoved(CVirtualDirectory *pDir, CVirtualDirectory *pOldDir, TString OldName);

    void DirectoryAboutToBeCreated(QString DirPath);
    void DirectoryCreated(CVirtualDirectory *pDir);

    void DirectoryAboutToBeDeleted(CVirtualDirectory *pDir);
    void DirectoryDeleted();
};

#endif // CRESOURCEBROWSER_H
