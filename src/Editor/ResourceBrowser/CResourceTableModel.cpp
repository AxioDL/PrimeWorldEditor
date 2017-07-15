#include "CResourceTableModel.h"
#include "CResourceBrowser.h"
#include "CResourceMimeData.h"

CResourceTableModel::CResourceTableModel(QObject *pParent /*= 0*/)
    : QAbstractTableModel(pParent)
    , mpCurrentDir(nullptr)
{
    connect(gpEdApp, SIGNAL(ResourceRenamed(CResourceEntry*)), this, SLOT(OnResourceRenamed(CResourceEntry*)));
    connect(gpEdApp, SIGNAL(DirectoryRenamed(CVirtualDirectory*)), this, SLOT(OnDirectoryRenamed(CVirtualDirectory*)));
}

// ************ INTERFACE ************
int CResourceTableModel::rowCount(const QModelIndex&) const
{
    return mDirectories.size() + mEntries.size();
}

int CResourceTableModel::columnCount(const QModelIndex&) const
{
    return 1;
}

QVariant CResourceTableModel::data(const QModelIndex& rkIndex, int Role) const
{
    if (rkIndex.column() != 0)
        return QVariant::Invalid;

    // Directory
    if (IsIndexDirectory(rkIndex))
    {
        CVirtualDirectory *pDir = IndexDirectory(rkIndex);

        if (Role == Qt::DisplayRole || Role == Qt::ToolTipRole)
            return (mHasParent && rkIndex.row() == 0 ? ".." : TO_QSTRING(pDir->Name()));

        else if (Role == Qt::DecorationRole)
            return QIcon(":/icons/Open_24px.png");

        else
            return QVariant::Invalid;
    }

    // Resource
    CResourceEntry *pEntry = IndexEntry(rkIndex);

    if (Role == Qt::DisplayRole)
        return TO_QSTRING(pEntry->Name());

    else if (Role == Qt::ToolTipRole)
        return TO_QSTRING(pEntry->CookedAssetPath(true));

    else if (Role == Qt::DecorationRole)
        return QIcon(":/icons/Sphere Preview.png");

    return QVariant::Invalid;
}

Qt::ItemFlags CResourceTableModel::flags(const QModelIndex& rkIndex) const
{
    Qt::ItemFlags Out = Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled;

    if (IsIndexDirectory(rkIndex))
        Out |= Qt::ItemIsDropEnabled;

    return Out;
}

bool CResourceTableModel::canDropMimeData(const QMimeData *pkData, Qt::DropAction, int Row, int Column, const QModelIndex& rkParent) const
{
    const CResourceMimeData *pkMimeData = qobject_cast<const CResourceMimeData*>(pkData);

    if (pkMimeData)
    {
        // Make sure we're dropping onto a directory
        QModelIndex Index = (rkParent.isValid() ? rkParent : index(Row, Column, rkParent));

        if (Index.isValid())
        {
            CVirtualDirectory *pDir = IndexDirectory(Index);

            if (pDir)
            {
                // Make sure this directory isn't part of the mime data, or a subdirectory of a directory in the mime data
                foreach (CVirtualDirectory *pMimeDir, pkMimeData->Directories())
                {
                    if (pDir == pMimeDir || pDir->IsDescendantOf(pMimeDir))
                        return false;
                }

                // Valid directory
                return true;
            }
        }
        else return false;
    }

    return false;
}

bool CResourceTableModel::dropMimeData(const QMimeData *pkData, Qt::DropAction, int Row, int Column, const QModelIndex& rkParent)
{
    const CResourceMimeData *pkMimeData = qobject_cast<const CResourceMimeData*>(pkData);

    QModelIndex Index = (rkParent.isValid() ? rkParent : index(Row, Column, rkParent));
    CVirtualDirectory *pDir = IndexDirectory(Index);
    ASSERT(pDir);

    gpEdApp->ResourceBrowser()->MoveResources( pkMimeData->Resources(), pkMimeData->Directories(), pDir );
    return true;
}

QMimeData* CResourceTableModel::mimeData(const QModelIndexList& rkIndexes) const
{
    if (rkIndexes.isEmpty())
        return nullptr;

    QList<CResourceEntry*> Resources;
    QList<CVirtualDirectory*> Dirs;

    foreach(QModelIndex Index, rkIndexes)
    {
        CResourceEntry *pEntry = IndexEntry(Index);
        CVirtualDirectory *pDir = IndexDirectory(Index);

        if (pEntry) Resources << pEntry;
        else        Dirs << pDir;
    }

    return new CResourceMimeData(Resources, Dirs);
}

Qt::DropActions CResourceTableModel::supportedDragActions() const
{
    return Qt::MoveAction | Qt::CopyAction;
}

Qt::DropActions CResourceTableModel::supportedDropActions() const
{
    return Qt::MoveAction;
}

// ************ FUNCTIONALITY ************
QModelIndex CResourceTableModel::GetIndexForEntry(CResourceEntry *pEntry) const
{
    if (mEntryIndexMap.contains(pEntry))
        return index(mEntryIndexMap[pEntry] + mDirectories.size(), 0, QModelIndex());
    else
        return QModelIndex();
}

CResourceEntry* CResourceTableModel::IndexEntry(const QModelIndex& rkIndex) const
{
    int Index = rkIndex.row() - mDirectories.size();
    return (Index >= 0 ? mEntries[Index] : nullptr);
}

CVirtualDirectory* CResourceTableModel::IndexDirectory(const QModelIndex& rkIndex) const
{
    return (IsIndexDirectory(rkIndex) ? mDirectories[rkIndex.row()] : nullptr);
}

bool CResourceTableModel::IsIndexDirectory(const QModelIndex& rkIndex) const
{
    return rkIndex.row() >= 0 && rkIndex.row() < mDirectories.size();
}

void CResourceTableModel::FillEntryList(CVirtualDirectory *pDir, bool AssetListMode)
{
    beginResetModel();

    mEntries.clear();
    mDirectories.clear();
    mEntryIndexMap.clear();
    mHasParent = false;

    if (pDir)
    {
        // In filesystem mode, show only subdirectories and assets in the current directory.
        if (!AssetListMode)
        {
            if (!pDir->IsRoot())
            {
                mDirectories << pDir->Parent();
                mHasParent = true;
            }

            for (u32 iDir = 0; iDir < pDir->NumSubdirectories(); iDir++)
                mDirectories << pDir->SubdirectoryByIndex(iDir);

            for (u32 iRes = 0; iRes < pDir->NumResources(); iRes++)
            {
                CResourceEntry *pEntry = pDir->ResourceByIndex(iRes);

                if (pEntry->TypeInfo()->IsVisibleInBrowser() && !pEntry->IsHidden())
                {
                    mEntryIndexMap[pEntry] = mEntries.size();
                    mEntries << pEntry;
                }
            }
        }

        // In asset list mode, do not show subdirectories and show all assets in current directory + all subdirectories.
        else
            RecursiveAddDirectoryContents(pDir);
    }

    endResetModel();
}

void CResourceTableModel::RecursiveAddDirectoryContents(CVirtualDirectory *pDir)
{
    for (u32 iRes = 0; iRes < pDir->NumResources(); iRes++)
    {
        CResourceEntry *pEntry = pDir->ResourceByIndex(iRes);

        if (pEntry->TypeInfo()->IsVisibleInBrowser() && !pEntry->IsHidden())
        {
            mEntryIndexMap[pEntry] = mEntries.size();
            mEntries << pEntry;
        }
    }

    for (u32 iDir = 0; iDir < pDir->NumSubdirectories(); iDir++)
        RecursiveAddDirectoryContents(pDir->SubdirectoryByIndex(iDir));
}

void CResourceTableModel::OnResourceRenamed(CResourceEntry *pEntry)
{
    if (mEntryIndexMap.contains(pEntry))
    {
        int Index = mEntries.indexOf(pEntry);
        int Row = Index + mDirectories.size();

        beginRemoveRows(QModelIndex(), Row, Row);
        mEntries.removeAt(Index);
        mEntryIndexMap.remove(pEntry);
        endRemoveRows();
    }
}

void CResourceTableModel::OnDirectoryRenamed(CVirtualDirectory *pDir)
{
    for (int DirIdx = 0; DirIdx < mDirectories.size(); DirIdx++)
    {
        if (mDirectories[DirIdx] == pDir)
        {
            beginRemoveRows(QModelIndex(), DirIdx, DirIdx);
            mDirectories.removeAt(DirIdx);
            endRemoveRows();
        }
    }
}
