#include "CVirtualDirectoryModel.h"
#include "CResourceBrowser.h"

CVirtualDirectoryModel::CVirtualDirectoryModel(CResourceBrowser *pBrowser, QObject *pParent /*= 0*/)
    : QAbstractItemModel(pParent)
    , mpRoot(nullptr)
    , mInsertingRows(false)
    , mRemovingRows(false)
    , mChangingLayout(false)
{
    connect(pBrowser, SIGNAL(DirectoryAboutToBeMoved(CVirtualDirectory*,QString)), this, SLOT(OnDirectoryAboutToBeMoved(CVirtualDirectory*,QString)));
    connect(pBrowser, SIGNAL(DirectoryMoved(CVirtualDirectory*,CVirtualDirectory*,TString)), this, SLOT(FinishModelChanges()));

    connect(pBrowser, SIGNAL(DirectoryAboutToBeCreated(QString)), this, SLOT(OnDirectoryAboutToBeCreated(QString)));
    connect(pBrowser, SIGNAL(DirectoryCreated(CVirtualDirectory*)), this, SLOT(FinishModelChanges()));

    connect(pBrowser, SIGNAL(DirectoryAboutToBeDeleted(CVirtualDirectory*)), this, SLOT(OnDirectoryAboutToBeDeleted(CVirtualDirectory*)));
    connect(pBrowser, SIGNAL(DirectoryDeleted()), this, SLOT(FinishModelChanges()));
}

QModelIndex CVirtualDirectoryModel::index(int Row, int Column, const QModelIndex& rkParent) const
{
    if (!hasIndex(Row, Column, rkParent))
        return QModelIndex();

    CVirtualDirectory *pDir = IndexDirectory(rkParent);

    if (pDir && pDir->NumSubdirectories() > (u32) Row)
        return createIndex(Row, Column, pDir->SubdirectoryByIndex(Row));

    else if (!pDir)
        return createIndex(Row, Column, mpRoot);

    return QModelIndex();
}

QModelIndex CVirtualDirectoryModel::parent(const QModelIndex& rkChild) const
{
    CVirtualDirectory *pDir = IndexDirectory(rkChild);
    CVirtualDirectory *pParent = pDir->Parent();

    if (pParent)
    {
        CVirtualDirectory *pGrandparent = pParent->Parent();

        if (pGrandparent)
        {
            for (u32 iSub = 0; iSub < pGrandparent->NumSubdirectories(); iSub++)
            {
                if (pGrandparent->SubdirectoryByIndex(iSub) == pParent)
                    return createIndex(iSub, 0, pParent);
            }
        }

        else return createIndex(0, 0, mpRoot);
    }

    return QModelIndex();
}

int CVirtualDirectoryModel::rowCount(const QModelIndex& rkParent) const
{
    CVirtualDirectory *pDir = IndexDirectory(rkParent);
    if (pDir) return pDir->NumSubdirectories();
    else return mpRoot ? 1 : 0;
}

int CVirtualDirectoryModel::columnCount(const QModelIndex&) const
{
    return 1;
}

QVariant CVirtualDirectoryModel::data(const QModelIndex& rkIndex, int Role) const
{
    if (!rkIndex.isValid())
        return QVariant::Invalid;

    if (Role == Qt::DisplayRole || Role == Qt::ToolTipRole)
    {
        if (!rkIndex.parent().isValid())
            return "Resources";

        else
        {
            CVirtualDirectory *pDir = IndexDirectory(rkIndex);
            if (pDir) return TO_QSTRING(pDir->Name());
        }
    }

    if (Role == Qt::DecorationRole)
    {
        return QIcon(":/icons/Open_24px.png");
    }

    return QVariant::Invalid;
}

QModelIndex CVirtualDirectoryModel::GetIndexForDirectory(CVirtualDirectory *pDir)
{
    if (!pDir)
        return QModelIndex();

    QVector<int> Indices;
    CVirtualDirectory *pOriginal = pDir;
    CVirtualDirectory *pParent = pDir->Parent();

    // Get index list
    while (pParent)
    {
        bool Found = false;

        for (u32 iDir = 0; iDir < pParent->NumSubdirectories(); iDir++)
        {
            if (pParent->SubdirectoryByIndex(iDir) == pDir)
            {
                Indices.push_front(iDir);
                pDir = pParent;
                pParent = pParent->Parent();
                Found = true;
                break;
            }
        }

        ASSERT(Found); // it should not be possible for this not to work
    }

    // Traverse hierarchy
    QModelIndex Out = index(0, 0, QModelIndex());

    foreach (int Idx, Indices)
        Out = index(Idx, 0, Out);

    ASSERT(IndexDirectory(Out) == pOriginal);
    return Out;
}

CVirtualDirectory* CVirtualDirectoryModel::IndexDirectory(const QModelIndex& rkIndex) const
{
    if (!rkIndex.isValid()) return nullptr;
    return static_cast<CVirtualDirectory*>(rkIndex.internalPointer());
}

void CVirtualDirectoryModel::SetRoot(CVirtualDirectory *pDir)
{
    beginResetModel();
    mpRoot = pDir;
    endResetModel();
}

bool CVirtualDirectoryModel::GetProposedIndex(QString Path, QModelIndex& rOutParent, int& rOutRow)
{
    // Get parent path
    TString FullPath = TO_TSTRING(Path);

    if (FullPath.EndsWith('/') || FullPath.EndsWith('\\'))
        FullPath = FullPath.ChopBack(1);

    u32 LastSlash = FullPath.LastIndexOf("\\/");
    TString ParentPath = FullPath.ChopBack( FullPath.Size() - LastSlash );

    // Find parent index
    CVirtualDirectory *pParent = (ParentPath.IsEmpty() ? mpRoot : mpRoot->FindChildDirectory(ParentPath, false));
    if (!pParent) return false;

    QModelIndex ParentIndex = GetIndexForDirectory(pParent);
    if (!ParentIndex.isValid()) return false;

    // Determine the row number that the new directory will be inserted at
    QString DirName = TO_QSTRING(FullPath.ChopFront( LastSlash + 1 ));
    int NumRows = rowCount(ParentIndex);
    int RowIdx = 0;

    for (; RowIdx < NumRows; RowIdx++)
    {
        QModelIndex Index = index(RowIdx, 0, ParentIndex);
        QString OtherName = data(Index, Qt::DisplayRole).toString();

        if (QString::compare(DirName, OtherName, Qt::CaseInsensitive) < 0)
            break;
    }

    rOutParent = ParentIndex;
    rOutRow = RowIdx;
    return true;
}

void CVirtualDirectoryModel::OnDirectoryAboutToBeMoved(CVirtualDirectory *, QString )
{
    emit layoutAboutToBeChanged();
    mChangingLayout = true;
}

void CVirtualDirectoryModel::OnDirectoryAboutToBeCreated(QString DirPath)
{
    QModelIndex Parent;
    int Row;

    if (GetProposedIndex(DirPath, Parent, Row))
    {
        beginInsertRows(Parent, Row, Row);
        mInsertingRows = true;
    }
}

void CVirtualDirectoryModel::OnDirectoryAboutToBeDeleted(CVirtualDirectory *pDir)
{
    QModelIndex Index = GetIndexForDirectory(pDir);

    if (Index.isValid())
    {
        beginRemoveRows(Index.parent(), Index.row(), Index.row());
        mRemovingRows = true;
    }
}

void CVirtualDirectoryModel::FinishModelChanges()
{
    if (mInsertingRows)
    {
        endInsertRows();
        mInsertingRows = false;
    }
    if (mRemovingRows)
    {
        endRemoveRows();
        mRemovingRows = false;
    }
    if (mChangingLayout)
    {
        emit layoutChanged();
        mChangingLayout = false;
    }
}
