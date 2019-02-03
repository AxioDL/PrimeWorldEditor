#include "CResourceTableModel.h"
#include "CResourceBrowser.h"
#include "CResourceMimeData.h"

CResourceTableModel::CResourceTableModel(CResourceBrowser *pBrowser, QObject *pParent /*= 0*/)
    : QAbstractTableModel(pParent)
    , mpCurrentDir(nullptr)
    , mIsDisplayingUserEntryList(false)
{
    connect(pBrowser, SIGNAL(ResourceCreated(CResourceEntry*)), this, SLOT(CheckAddResource(CResourceEntry*)));
    connect(pBrowser, SIGNAL(ResourceAboutToBeDeleted(CResourceEntry*)), this, SLOT(CheckRemoveResource(CResourceEntry*)));
    connect(pBrowser, SIGNAL(DirectoryCreated(CVirtualDirectory*)), this, SLOT(CheckAddDirectory(CVirtualDirectory*)));
    connect(pBrowser, SIGNAL(DirectoryAboutToBeDeleted(CVirtualDirectory*)), this, SLOT(CheckRemoveDirectory(CVirtualDirectory*)));
    connect(pBrowser, SIGNAL(ResourceMoved(CResourceEntry*,CVirtualDirectory*,TString)), this, SLOT(OnResourceMoved(CResourceEntry*,CVirtualDirectory*,TString)));
    connect(pBrowser, SIGNAL(DirectoryMoved(CVirtualDirectory*,CVirtualDirectory*,TString)), this, SLOT(OnDirectoryMoved(CVirtualDirectory*,CVirtualDirectory*,TString)));
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
            return ( (mpCurrentDir && !mpCurrentDir->IsRoot() && rkIndex.row() == 0)
                    ? ".." : TO_QSTRING(pDir->Name()));

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
    Qt::ItemFlags Out = Qt::ItemIsSelectable |Qt::ItemIsEnabled;
    CVirtualDirectory *pDir = IndexDirectory(rkIndex);

    if (pDir)
        Out |= Qt::ItemIsDropEnabled;

    if (!pDir || mpCurrentDir->Parent() != pDir)
        Out |= Qt::ItemIsEditable | Qt::ItemIsDragEnabled;

    return Out;
}

bool CResourceTableModel::canDropMimeData(const QMimeData *pkData, Qt::DropAction, int Row, int Column, const QModelIndex& rkParent) const
{
    // Don't allow dropping between items.
    if (Row != -1 || Column != -1)
        return false;

    const CResourceMimeData *pkMimeData = qobject_cast<const CResourceMimeData*>(pkData);

    if (pkMimeData)
    {
        // Don't allow dropping onto blank space in asset list mode
        if (!rkParent.isValid() && mIsAssetListMode)
            return false;

        // Make sure we're dropping onto a directory
        CVirtualDirectory *pDir = (rkParent.isValid() ? IndexDirectory(rkParent) : mpCurrentDir);

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

    return false;
}

bool CResourceTableModel::dropMimeData(const QMimeData *pkData, Qt::DropAction Action, int Row, int Column, const QModelIndex& rkParent)
{
    const CResourceMimeData *pkMimeData = qobject_cast<const CResourceMimeData*>(pkData);

    if (canDropMimeData(pkData, Action, Row, Column, rkParent))
    {
        CVirtualDirectory *pDir = (rkParent.isValid() ? IndexDirectory(rkParent) : mpCurrentDir);
        ASSERT(pDir);

        return gpEdApp->ResourceBrowser()->MoveResources( pkMimeData->Resources(), pkMimeData->Directories(), pDir );
    }
    else return false;
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
    auto Iter = std::find(mEntries.begin(), mEntries.end(), pEntry);

    if (Iter == mEntries.end())
        return QModelIndex();

    else
    {
        int Index = Iter - mEntries.begin();
        return index(mDirectories.size() + Index, 0, QModelIndex());
    }
}

QModelIndex CResourceTableModel::GetIndexForDirectory(CVirtualDirectory *pDir) const
{
    for (int DirIdx = 0; DirIdx < mDirectories.size(); DirIdx++)
    {
        if (mDirectories[DirIdx] == pDir)
            return index(DirIdx, 0, QModelIndex());
    }

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

bool CResourceTableModel::HasParentDirectoryEntry() const
{
    return !mIsAssetListMode && mpCurrentDir && !mpCurrentDir->IsRoot();
}

void CResourceTableModel::FillEntryList(CVirtualDirectory *pDir, bool AssetListMode)
{
    beginResetModel();

    mpCurrentDir = pDir;
    mEntries.clear();
    mDirectories.clear();
    mIsAssetListMode = AssetListMode;
    mIsDisplayingUserEntryList = false;

    if (pDir)
    {
        // In filesystem mode, show only subdirectories and assets in the current directory.
        if (!mIsAssetListMode)
        {
            if (!pDir->IsRoot())
                mDirectories << pDir->Parent();

            for (uint32 iDir = 0; iDir < pDir->NumSubdirectories(); iDir++)
                mDirectories << pDir->SubdirectoryByIndex(iDir);

            for (uint32 iRes = 0; iRes < pDir->NumResources(); iRes++)
            {
                CResourceEntry *pEntry = pDir->ResourceByIndex(iRes);

                if (!pEntry->IsHidden())
                {
                    int Index = EntryListIndex(pEntry);
                    mEntries.insert(Index, pEntry);
                }
            }
        }

        // In asset list mode, do not show subdirectories and show all assets in current directory + all subdirectories.
        else
            RecursiveAddDirectoryContents(pDir);
    }

    if (pDir)
        mModelDescription = pDir->IsRoot() ? "Resources" : TO_QSTRING(pDir->FullPath());
    else
        mModelDescription = "Nothing";

    endResetModel();
}

void CResourceTableModel::DisplayEntryList(QList<CResourceEntry*>& rkEntries, const QString& rkListDescription)
{
    beginResetModel();
    mEntries = rkEntries;
    mDirectories.clear();
    mModelDescription = rkListDescription;
    mIsAssetListMode = true;
    mIsDisplayingUserEntryList = true;
    endResetModel();
}

void CResourceTableModel::RecursiveAddDirectoryContents(CVirtualDirectory *pDir)
{
    for (uint32 iRes = 0; iRes < pDir->NumResources(); iRes++)
    {
        CResourceEntry *pEntry = pDir->ResourceByIndex(iRes);

        if (!pEntry->IsHidden())
        {
            int Index = EntryListIndex(pEntry);
            mEntries.insert(Index, pEntry);
        }
    }

    for (uint32 iDir = 0; iDir < pDir->NumSubdirectories(); iDir++)
        RecursiveAddDirectoryContents(pDir->SubdirectoryByIndex(iDir));
}

int CResourceTableModel::EntryListIndex(CResourceEntry *pEntry)
{
    return qLowerBound(mEntries, pEntry) - mEntries.constBegin();
}

void CResourceTableModel::RefreshAllIndices()
{
    int NumRows = rowCount(QModelIndex());
    int NumCols = columnCount(QModelIndex());

    if (NumRows > 0 && NumCols > 0)
    {
        emit dataChanged( index(0,0), index(NumRows-1, NumCols-1) );
    }
}

void CResourceTableModel::CheckAddResource(CResourceEntry *pEntry)
{
    if ( (mIsAssetListMode && pEntry->IsInDirectory(mpCurrentDir)) ||
         (!mIsAssetListMode && pEntry->Directory() == mpCurrentDir) )
    {
        // Append to the end, let the proxy handle sorting
        int NumRows = mDirectories.size() + mEntries.size();
        beginInsertRows(QModelIndex(), NumRows, NumRows);
        mEntries << pEntry;
        endInsertRows();
    }
}

void CResourceTableModel::CheckRemoveResource(CResourceEntry *pEntry)
{
    int Index = mEntries.indexOf(pEntry);

    if (Index != -1)
    {
        int RowIndex = Index + mDirectories.size();
        beginRemoveRows(QModelIndex(), RowIndex, RowIndex);
        mEntries.removeAt(Index);
        endRemoveRows();
    }
}

void CResourceTableModel::CheckAddDirectory(CVirtualDirectory *pDir)
{
    if (pDir->Parent() == mpCurrentDir)
    {
        // Just append to the end, let the proxy handle sorting
        beginInsertRows(QModelIndex(), mDirectories.size(), mDirectories.size());
        mDirectories << pDir;
        endInsertRows();
    }
}

void CResourceTableModel::CheckRemoveDirectory(CVirtualDirectory *pDir)
{
    int Index = mDirectories.indexOf(pDir);

    if (Index != -1)
    {
        beginRemoveRows(QModelIndex(), Index, Index);
        mDirectories.removeAt(Index);
        endRemoveRows();
    }
}

void CResourceTableModel::OnResourceMoved(CResourceEntry *pEntry, CVirtualDirectory *pOldDir, TString OldName)
{
    CVirtualDirectory *pNewDir = pEntry->Directory();
    bool WasInModel = (pOldDir == mpCurrentDir || (mIsAssetListMode && pOldDir->IsDescendantOf(mpCurrentDir)));
    bool IsInModel = (pNewDir == mpCurrentDir || (mIsAssetListMode && pNewDir->IsDescendantOf(mpCurrentDir)));

    // Handle rename
    if (WasInModel && IsInModel && pEntry->Name() != OldName)
    {
        int ResIdx = EntryListIndex(pEntry);
        int Row = ResIdx + mDirectories.size();
        QModelIndex Index = index(Row, 0, QModelIndex());
        emit dataChanged(Index, Index);
    }

    else if (pNewDir != pOldDir)
    {
        // Remove
        if (WasInModel && !IsInModel)
        {
            int Pos = EntryListIndex(pEntry);
            int Row = mDirectories.size() + Pos;

            beginRemoveRows(QModelIndex(), Row, Row);
            mEntries.removeAt(Pos);
            endRemoveRows();
        }

        // Add
        else if (!WasInModel && IsInModel)
        {
            int Index = EntryListIndex(pEntry);
            int Row = mDirectories.size() + Index;

            beginInsertRows(QModelIndex(), Row, Row);
            mEntries.insert(Index, pEntry);
            endInsertRows();
        }
    }
}

void CResourceTableModel::OnDirectoryMoved(CVirtualDirectory *pDir, CVirtualDirectory *pOldDir, TString OldName)
{
    CVirtualDirectory *pNewDir = pDir->Parent();
    bool WasInModel = !mIsAssetListMode && pOldDir == mpCurrentDir;
    bool IsInModel = !mIsAssetListMode && pNewDir == mpCurrentDir;

    // Handle parent link
    if (pDir == mpCurrentDir)
    {
        ASSERT(mDirectories.front() == pOldDir);
        mDirectories[0] = pNewDir;
    }

    // Handle rename
    if (WasInModel && IsInModel && pDir->Name() != OldName)
    {
        QModelIndex Index = GetIndexForDirectory(pDir);
        emit dataChanged(Index, Index);
    }

    else if (pNewDir != pOldDir)
    {
        // Remove
        if (WasInModel && !IsInModel)
            CheckRemoveDirectory(pDir);

        // Add
        else if (!WasInModel && IsInModel)
            CheckAddDirectory(pDir);
    }
}
