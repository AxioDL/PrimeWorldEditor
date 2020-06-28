#ifndef CVIRTUALDIRECTORYMODEL
#define CVIRTUALDIRECTORYMODEL

#include "Editor/UICommon.h"
#include <Core/GameProject/CVirtualDirectory.h>
#include <QAbstractItemModel>
#include <QIcon>

class CVirtualDirectoryModel : public QAbstractItemModel
{
    Q_OBJECT
    CVirtualDirectory *mpRoot = nullptr;
    bool mInsertingRows = false;
    bool mRemovingRows = false;
    bool mMovingRows = false;
    bool mChangingLayout = false;

public:
    explicit CVirtualDirectoryModel(CResourceBrowser *pBrowser, QObject *pParent = nullptr);

    QModelIndex index(int Row, int Column, const QModelIndex& rkParent) const override;
    QModelIndex parent(const QModelIndex& rkChild) const override;
    int rowCount(const QModelIndex& rkParent) const override;
    int columnCount(const QModelIndex& /*rkParent*/) const override;
    QVariant data(const QModelIndex& rkIndex, int Role) const override;
    bool setData(const QModelIndex& rkIndex, const QVariant& rkValue, int Role) override;
    Qt::ItemFlags flags(const QModelIndex& rkIndex) const override;

    bool canDropMimeData(const QMimeData *pkData, Qt::DropAction Action, int Row, int Column, const QModelIndex& rkParent) const override;
    bool dropMimeData(const QMimeData *pkData, Qt::DropAction Action, int Row, int Column, const QModelIndex& rkParent) override;
    QMimeData* mimeData(const QModelIndexList& rkIndexes) const override;
    Qt::DropActions supportedDragActions() const override;
    Qt::DropActions supportedDropActions() const override;

    QModelIndex GetIndexForDirectory(CVirtualDirectory *pDir);
    CVirtualDirectory* IndexDirectory(const QModelIndex& rkIndex) const;
    void SetRoot(CVirtualDirectory *pDir);

protected:
    bool GetProposedIndex(QString Path, QModelIndex& rOutParent, int& rOutRow);

public slots:
    void OnDirectoryAboutToBeMoved(CVirtualDirectory *pDir, QString NewPath);
    void OnDirectoryAboutToBeCreated(QString DirPath);
    void OnDirectoryAboutToBeDeleted(CVirtualDirectory *pDir);
    void FinishModelChanges();
};

#endif // CVIRTUALDIRECTORYMODEL

