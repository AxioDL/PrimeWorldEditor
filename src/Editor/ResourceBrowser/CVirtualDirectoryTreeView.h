#ifndef CVIRTUALDIRECTORYTREEVIEW_H
#define CVIRTUALDIRECTORYTREEVIEW_H

#include <QTreeView>
#include "CVirtualDirectoryModel.h"
#include <Core/GameProject/CVirtualDirectory.h>

class CVirtualDirectoryTreeView : public QTreeView
{
    Q_OBJECT

    CVirtualDirectoryModel *mpModel;
    bool mTransferSelectionPostMove;

public:
    CVirtualDirectoryTreeView(QWidget *pParent = 0);
    void dragEnterEvent(QDragEnterEvent *pEvent);
    void setModel(QAbstractItemModel *pModel);

public slots:
    void OnDirectoryAboutToBeMoved(CVirtualDirectory *pDir);
    void OnDirectoryMoved(CVirtualDirectory *pDir);
};

#endif // CVIRTUALDIRECTORYTREEVIEW_H
