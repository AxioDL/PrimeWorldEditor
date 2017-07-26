#ifndef CRESOURCETABLEVIEW_H
#define CRESOURCETABLEVIEW_H

#include <QTableView>
#include "CResourceTableModel.h"
#include "CResourceProxyModel.h"

class CResourceTableView : public QTableView
{
    Q_OBJECT

    CResourceTableModel *mpModel;
    CResourceProxyModel *mpProxy;
    QAction *mpDeleteAction;

public:
    explicit CResourceTableView(QWidget *pParent = 0);
    void setModel(QAbstractItemModel *pModel);
    void dragEnterEvent(QDragEnterEvent *pEvent);

public slots:
    void DeleteSelected();
};

#endif // CRESOURCETABLEVIEW_H
