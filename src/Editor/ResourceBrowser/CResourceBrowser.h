#ifndef CRESOURCEBROWSER_H
#define CRESOURCEBROWSER_H

#include "CResourceProxyModel.h"
#include "CResourceTableModel.h"
#include "CVirtualDirectoryModel.h"
#include <QDialog>

namespace Ui {
class CResourceBrowser;
}

class CResourceBrowser : public QDialog
{
    Q_OBJECT
    Ui::CResourceBrowser *mpUI;
    CResourceTableModel *mpModel;
    CResourceProxyModel *mpProxyModel;
    CVirtualDirectoryModel *mpDirectoryModel;

public:
    explicit CResourceBrowser(QWidget *pParent = 0);
    ~CResourceBrowser();
    void RefreshResources();

public slots:
    void OnSortModeChanged(int Index);
    void OnSearchStringChanged();
    void OnDirectorySelectionChanged(const QModelIndex& rkNewIndex, const QModelIndex& rkPrevIndex);
};

#endif // CRESOURCEBROWSER_H
