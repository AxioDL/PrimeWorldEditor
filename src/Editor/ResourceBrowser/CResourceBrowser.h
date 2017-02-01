#ifndef CRESOURCEBROWSER_H
#define CRESOURCEBROWSER_H

#include "CResourceProxyModel.h"
#include "CResourceTableModel.h"
#include "CVirtualDirectoryModel.h"
#include <QDialog>
#include <QTimer>

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
    CVirtualDirectoryModel *mpDirectoryModel;
    QTimer mUpdateFilterTimer;

public:
    explicit CResourceBrowser(QWidget *pParent = 0);
    ~CResourceBrowser();

    // Accessors
    inline CResourceEntry* SelectedEntry() const    { return mpSelectedEntry; }

public slots:
    void RefreshResources();
    void OnSortModeChanged(int Index);
    void OnSearchStringChanged();
    void OnDirectorySelectionChanged(const QModelIndex& rkNewIndex, const QModelIndex& rkPrevIndex);
    void OnDoubleClickResource(QModelIndex Index);
    void OnResourceSelectionChanged(const QModelIndex& rkNewIndex, const QModelIndex& rkPrevIndex);
    void OnImportPakContentsTxt();
    void OnGenerateAssetNames();
    void OnImportNamesFromAssetNameMap();
    void ExportAssetNames();
    void UpdateFilter();

signals:
    void SelectedResourceChanged(CResourceEntry *pNewRes);
};

#endif // CRESOURCEBROWSER_H
