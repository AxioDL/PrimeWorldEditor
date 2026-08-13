#ifndef CVIRTUALDIRECTORYMODEL_H
#define CVIRTUALDIRECTORYMODEL_H

#include <QAbstractItemModel>

#include <optional>
#include <utility>

class CResourceBrowser;
class CVirtualDirectory;

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
    ~CVirtualDirectoryModel() override;

    QModelIndex index(int Row, int Column, const QModelIndex& rkParent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& rkChild) const override;
    int rowCount(const QModelIndex& rkParent = QModelIndex()) const override;
    int columnCount(const QModelIndex& = QModelIndex()) const override;
    QVariant data(const QModelIndex& rkIndex, int Role) const override;
    bool setData(const QModelIndex& rkIndex, const QVariant& rkValue, int Role) override;
    Qt::ItemFlags flags(const QModelIndex& rkIndex) const override;

    bool canDropMimeData(const QMimeData *pkData, Qt::DropAction Action, int Row, int Column, const QModelIndex& rkParent) const override;
    bool dropMimeData(const QMimeData *pkData, Qt::DropAction Action, int Row, int Column, const QModelIndex& rkParent) override;
    QMimeData* mimeData(const QModelIndexList& rkIndexes) const override;
    Qt::DropActions supportedDragActions() const override;
    Qt::DropActions supportedDropActions() const override;

    QModelIndex GetIndexForDirectory(const CVirtualDirectory *pDir) const;
    CVirtualDirectory* IndexDirectory(const QModelIndex& rkIndex) const;
    void SetRoot(CVirtualDirectory *pDir);

public slots:
    void FinishModelChanges();

protected:
    std::optional<std::pair<QModelIndex, int>> GetProposedIndex(const QString& Path) const;

private slots:
    void OnDirectoryAboutToBeMoved(const CVirtualDirectory *pDir, const QString& NewPath);
    void OnDirectoryAboutToBeCreated(const QString& DirPath);
    void OnDirectoryAboutToBeDeleted(const CVirtualDirectory *pDir);
};

#endif // CVIRTUALDIRECTORYMODEL_H

