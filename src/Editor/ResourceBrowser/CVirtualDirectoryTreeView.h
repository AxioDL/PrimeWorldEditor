#ifndef CVIRTUALDIRECTORYTREEVIEW_H
#define CVIRTUALDIRECTORYTREEVIEW_H

#include <QTreeView>
#include "CVirtualDirectoryModel.h"
#include <Core/GameProject/CVirtualDirectory.h>

class CVirtualDirectoryTreeView : public QTreeView
{
    Q_OBJECT

    CVirtualDirectoryModel *mpModel = nullptr;
    bool mTransferSelectionPostMove = false;

public:
    explicit CVirtualDirectoryTreeView(QWidget *pParent = nullptr);

    void dragEnterEvent(QDragEnterEvent *pEvent) override;
    void setModel(QAbstractItemModel *pModel) override;

public slots:
    void OnDirectoryAboutToBeMoved(const CVirtualDirectory *pDir);
    void OnDirectoryMoved(const CVirtualDirectory *pDir);
};

#endif // CVIRTUALDIRECTORYTREEVIEW_H
