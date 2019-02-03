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
    Ui::CResourceBrowser *mpUI;
    CResourceEntry *mpSelectedEntry;
    CResourceStore *mpStore;
    CResourceTableModel *mpModel;
    CResourceProxyModel *mpProxyModel;
    CResourceBrowserDelegate *mpDelegate;
    CVirtualDirectory *mpSelectedDir;
    CVirtualDirectoryModel *mpDirectoryModel;
    bool mEditorStore;
    bool mAssetListMode;
    bool mSearching;

    // Add Menu
    QMenu *mpAddMenu;

    // Type Filter
    QWidget *mpFilterBoxesContainerWidget;
    QVBoxLayout *mpFilterBoxesLayout;
    QCheckBox *mpFilterAllBox;
    QFont mFilterBoxFont;

    struct SResourceType
    {
        CResTypeInfo *pTypeInfo;
        QCheckBox *pFilterCheckBox;
    };
    QList<SResourceType> mTypeList;

    // Undo/Redo
    QUndoStack mUndoStack;
    QAction *mpUndoAction;
    QAction *mpRedoAction;
    QWidget *mpActionContainerWidget;

    // Misc
    CResourceEntry *mpInspectedEntry; // Entry being "inspected" (viewing dependencies/referencers, etc)

public:
    explicit CResourceBrowser(QWidget *pParent = 0);
    ~CResourceBrowser();

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
    bool eventFilter(QObject *pWatched, QEvent *pEvent);

    // Accessors
    inline CResourceStore* CurrentStore() const     { return mpStore; }
    inline CResourceEntry* SelectedEntry() const    { return mpSelectedEntry; }
    inline bool InAssetListMode() const             { return mAssetListMode || mSearching || mpModel->IsDisplayingUserEntryList(); }

    inline void SetInspectedEntry(CResourceEntry *pEntry)   { mpInspectedEntry = pEntry; }

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
    void OnSearchStringChanged(QString SearchString);
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
