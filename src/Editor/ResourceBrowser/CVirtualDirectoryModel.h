#ifndef CVIRTUALDIRECTORYMODEL
#define CVIRTUALDIRECTORYMODEL

#include "Editor/UICommon.h"
#include <Core/GameProject/CVirtualDirectory.h>
#include <QAbstractItemModel>
#include <QIcon>

class CVirtualDirectoryModel : public QAbstractItemModel
{
    Q_OBJECT
    CVirtualDirectory *mpRoot;
    bool mInsertingRows;
    bool mRemovingRows;
    bool mChangingLayout;

public:
    CVirtualDirectoryModel(CResourceBrowser *pBrowser, QObject *pParent = 0);

    QModelIndex index(int Row, int Column, const QModelIndex& rkParent) const;
    QModelIndex parent(const QModelIndex& rkChild) const;
    int rowCount(const QModelIndex& rkParent) const;
    int columnCount(const QModelIndex& /*rkParent*/) const;
    QVariant data(const QModelIndex& rkIndex, int Role) const;

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

