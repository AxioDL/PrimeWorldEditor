#ifndef CRESOURCEBROWSER_H
#define CRESOURCEBROWSER_H

#include "CResourceProxyModel.h"
#include "CResourceTableModel.h"
#include "CVirtualDirectoryModel.h"
#include <QCheckBox>
#include <QDialog>
#include <QTimer>
#include <QVBoxLayout>

namespace Ui {
class CResourceBrowser;
}

class CResourceBrowser : public QDialog
{
    Q_OBJECT
    Ui::CResourceBrowser *mpUI;
    CResourceEntry *mpSelectedEntry;
    CResourceStore *mpStore;
    CResourceTableModel *mpModel;
    CResourceProxyModel *mpProxyModel;
    CVirtualDirectory *mpSelectedDir;
    CVirtualDirectoryModel *mpDirectoryModel;
    QTimer mUpdateFilterTimer;
    bool mAssetListMode;
    bool mSearching;

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

public:
    explicit CResourceBrowser(QWidget *pParent = 0);
    ~CResourceBrowser();

    void SelectResource(CResourceEntry *pEntry);
    void SelectDirectory(CVirtualDirectory *pDir);
    void CreateFilterCheckboxes();

    // Accessors
    inline CResourceEntry* SelectedEntry() const    { return mpSelectedEntry; }
    inline bool InAssetListMode() const             { return mAssetListMode || mSearching; }

public slots:
    void RefreshResources();
    void RefreshDirectories();
    void UpdateDescriptionLabel();
    void UpdateStore();
    void OnDisplayModeChanged(int Index);
    void OnSortModeChanged(int Index);
    void OnSearchStringChanged();
    void OnDirectorySelectionChanged(const QModelIndex& rkNewIndex, const QModelIndex& rkPrevIndex);
    void OnDoubleClickTable(QModelIndex Index);
    void OnResourceSelectionChanged(const QModelIndex& rkNewIndex, const QModelIndex& rkPrevIndex);
    void OnImportPakContentsTxt();
    void OnGenerateAssetNames();
    void OnImportNamesFromAssetNameMap();
    void ExportAssetNames();
    void UpdateFilter();

    void ResetTypeFilter();
    void OnFilterTypeBoxTicked(bool Checked);

signals:
    void SelectedResourceChanged(CResourceEntry *pNewRes);
};

#endif // CRESOURCEBROWSER_H
