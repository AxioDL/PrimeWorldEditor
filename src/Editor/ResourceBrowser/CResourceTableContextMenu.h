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

    QModelIndexList mSelectedIndexes;
    QModelIndex mClickedIndex;
    QModelIndex mClickedProxyIndex;
    CResourceEntry *mpClickedEntry = nullptr;
    CVirtualDirectory *mpClickedDirectory = nullptr;

public:
    CResourceTableContextMenu(CResourceBrowser *pBrowser, QTableView *pView, CResourceTableModel *pModel, CResourceProxyModel *pProxy);

public slots:
    void InitMenu();
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

    // Asset Specific
    void CreateSCAN();
};

#endif // CRESOURCETABLECONTEXTMENU_H
