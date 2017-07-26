#ifndef CRESOURCETABLECONTEXTMENU_H
#define CRESOURCETABLECONTEXTMENU_H

#include "CResourceTableModel.h"
#include "CResourceProxyModel.h"

#include <QMenu>
#include <QTableView>

class CResourceTableContextMenu : public QMenu
{
    Q_OBJECT

    CResourceBrowser *mpBrowser;
    QTableView *mpTable;
    CResourceTableModel *mpModel;
    CResourceProxyModel *mpProxy;

    QModelIndex mProxyIndex;
    QModelIndex mIndex;
    CResourceEntry *mpEntry;
    CVirtualDirectory *mpDirectory;

    // Actions
    QAction *mpOpenAction;
    QAction *mpOpenInExternalAppAction;
    QAction *mpOpenInExplorerAction;
    QAction *mpSelectFolderAction;

    QAction *mpRenameAction;
    QAction *mpShowReferencersAction;
    QAction *mpShowDependenciesAction;
    QAction *mpDeleteAction;

    QAction *mpCopyNameAction;
    QAction *mpCopyPathAction;
    QAction *mpCopyIDAction;

public:
    CResourceTableContextMenu(CResourceBrowser *pBrowser, QTableView *pView, CResourceTableModel *pModel, CResourceProxyModel *pProxy);

public slots:
    void ShowMenu(const QPoint& rkPos);

    // Menu Options
    void Open();
    void OpenInExternalApp();
    void OpenInExplorer();
    void Rename();
    void SelectFolder();
    void ShowReferencers();
    void ShowDependencies();
    void Delete();
    void CopyName();
    void CopyPath();
    void CopyID();
};

#endif // CRESOURCETABLECONTEXTMENU_H
