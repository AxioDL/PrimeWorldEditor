#include "CResourceTableModel.h"
#include "CResourceBrowser.h"
#include "CResourceMimeData.h"

CResourceTableModel::CResourceTableModel(CResourceBrowser *pBrowser, QObject *pParent)
    : QAbstractTableModel(pParent)
{
    connect(pBrowser, &CResourceBrowser::ResourceCreated, this, &CResourceTableModel::CheckAddResource);
    connect(pBrowser, &CResourceBrowser::ResourceAboutToBeDeleted, this, &CResourceTableModel::CheckRemoveResource);
    connect(pBrowser, &CResourceBrowser::DirectoryCreated, this, &CResourceTableModel::CheckAddDirectory);
    connect(pBrowser, &CResourceBrowser::DirectoryAboutToBeDeleted, this, &CResourceTableModel::CheckRemoveDirectory);
    connect(pBrowser, &CResourceBrowser::ResourceMoved, this, &CResourceTableModel::OnResourceMoved);
    connect(pBrowser, &CResourceBrowser::DirectoryMoved, this, &CResourceTableModel::OnDirectoryMoved);
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
        {
            return ((mpCurrentDir && !mpCurrentDir->IsRoot() && rkIndex.row() == 0)
                        ? tr("..")
                        : TO_QSTRING(pDir->Name()));
        }

        if (Role == Qt::DecorationRole)
            return QIcon(QStringLiteral(":/icons/Open_24px.svg"));

        return QVariant::Invalid;
    }

    // Resource
    CResourceEntry *pEntry = IndexEntry(rkIndex);

    if (Role == Qt::DisplayRole)
        return TO_QSTRING(pEntry->Name());

    if (Role == Qt::ToolTipRole)
        return TO_QSTRING(pEntry->CookedAssetPath(true));

    if (Role == Qt::DecorationRole)
        return QIcon(QStringLiteral(":/icons/Sphere Preview.svg"));

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
            for (CVirtualDirectory *pMimeDir : pkMimeData->Directories())
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
    return false;
}

QMimeData* CResourceTableModel::mimeData(const QModelIndexList& rkIndexes) const
{
    if (rkIndexes.isEmpty())
        return nullptr;

    QList<CResourceEntry*> Resources;
    QList<CVirtualDirectory*> Dirs;

    for (const QModelIndex Index : rkIndexes)
    {
        CResourceEntry *pEntry = IndexEntry(Index);
        CVirtualDirectory *pDir = IndexDirectory(Index);

        if (pEntry)
            Resources.push_back(pEntry);
        else
            Dirs.push_back(pDir);
    }

    return new CResourceMimeData(std::move(Resources), std::move(Dirs));
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
    const auto Iter = std::find(mEntries.begin(), mEntries.end(), pEntry);

    if (Iter == mEntries.cend())
        return QModelIndex();

    const int Index = std::distance(mEntries.begin(), Iter);
    return index(mDirectories.size() + Index, 0, QModelIndex());
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
    const int Index = rkIndex.row() - mDirectories.size();
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
                mDirectories.push_back(pDir->Parent());

            for (size_t iDir = 0; iDir < pDir->NumSubdirectories(); iDir++)
                mDirectories.push_back(pDir->SubdirectoryByIndex(iDir));

            for (size_t iRes = 0; iRes < pDir->NumResources(); iRes++)
            {
                CResourceEntry *pEntry = pDir->ResourceByIndex(iRes);

                if (!pEntry->IsHidden())
                {
                    const int Index = EntryListIndex(pEntry);
                    mEntries.insert(Index, pEntry);
                }
            }
        }
        // In asset list mode, do not show subdirectories and show all assets in current directory + all subdirectories.
        else
        {
            RecursiveAddDirectoryContents(pDir);
        }
    }

    if (pDir)
        mModelDescription = pDir->IsRoot() ? tr("Resources") : TO_QSTRING(pDir->FullPath());
    else
        mModelDescription = tr("Nothing");

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
    for (size_t iRes = 0; iRes < pDir->NumResources(); iRes++)
    {
        CResourceEntry *pEntry = pDir->ResourceByIndex(iRes);

        if (!pEntry->IsHidden())
        {
            const int Index = EntryListIndex(pEntry);
            mEntries.insert(Index, pEntry);
        }
    }

    for (size_t iDir = 0; iDir < pDir->NumSubdirectories(); iDir++)
        RecursiveAddDirectoryContents(pDir->SubdirectoryByIndex(iDir));
}

int CResourceTableModel::EntryListIndex(CResourceEntry *pEntry)
{
    return qLowerBound(mEntries, pEntry) - mEntries.constBegin();
}

void CResourceTableModel::RefreshAllIndices()
{
    const int NumRows = rowCount(QModelIndex());
    const int NumCols = columnCount(QModelIndex());

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
        const int NumRows = mDirectories.size() + mEntries.size();
        beginInsertRows(QModelIndex(), NumRows, NumRows);
        mEntries.push_back(pEntry);
        endInsertRows();
    }
}

void CResourceTableModel::CheckRemoveResource(CResourceEntry *pEntry)
{
    const int Index = mEntries.indexOf(pEntry);

    if (Index == -1)
        return;

    const int RowIndex = Index + mDirectories.size();
    beginRemoveRows(QModelIndex(), RowIndex, RowIndex);
    mEntries.removeAt(Index);
    endRemoveRows();
}

void CResourceTableModel::CheckAddDirectory(CVirtualDirectory *pDir)
{
    if (pDir->Parent() != mpCurrentDir)
        return;

    // Just append to the end, let the proxy handle sorting
    beginInsertRows(QModelIndex(), mDirectories.size(), mDirectories.size());
    mDirectories.push_back(pDir);
    endInsertRows();
}

void CResourceTableModel::CheckRemoveDirectory(CVirtualDirectory *pDir)
{
    const int Index = mDirectories.indexOf(pDir);

    if (Index == -1)
        return;

    beginRemoveRows(QModelIndex(), Index, Index);
    mDirectories.removeAt(Index);
    endRemoveRows();
}

void CResourceTableModel::OnResourceMoved(CResourceEntry *pEntry, CVirtualDirectory *pOldDir, TString OldName)
{
    CVirtualDirectory *pNewDir = pEntry->Directory();
    const bool WasInModel = (pOldDir == mpCurrentDir || (mIsAssetListMode && pOldDir->IsDescendantOf(mpCurrentDir)));
    const bool IsInModel = (pNewDir == mpCurrentDir || (mIsAssetListMode && pNewDir->IsDescendantOf(mpCurrentDir)));

    // Handle rename
    if (WasInModel && IsInModel && pEntry->Name() != OldName)
    {
        const int ResIdx = EntryListIndex(pEntry);
        const int Row = ResIdx + mDirectories.size();
        const QModelIndex Index = index(Row, 0, QModelIndex());
        emit dataChanged(Index, Index);
    }
    else if (pNewDir != pOldDir)
    {
        // Remove
        if (WasInModel && !IsInModel)
        {
            const int Pos = EntryListIndex(pEntry);
            const int Row = mDirectories.size() + Pos;

            beginRemoveRows(QModelIndex(), Row, Row);
            mEntries.removeAt(Pos);
            endRemoveRows();
        }
        // Add
        else if (!WasInModel && IsInModel)
        {
            const int Index = EntryListIndex(pEntry);
            const int Row = mDirectories.size() + Index;

            beginInsertRows(QModelIndex(), Row, Row);
            mEntries.insert(Index, pEntry);
            endInsertRows();
        }
    }
}

void CResourceTableModel::OnDirectoryMoved(CVirtualDirectory *pDir, CVirtualDirectory *pOldDir, TString OldName)
{
    CVirtualDirectory *pNewDir = pDir->Parent();
    const bool WasInModel = !mIsAssetListMode && pOldDir == mpCurrentDir;
    const bool IsInModel = !mIsAssetListMode && pNewDir == mpCurrentDir;

    // Handle parent link
    if (pDir == mpCurrentDir)
    {
        ASSERT(mDirectories.front() == pOldDir);
        mDirectories[0] = pNewDir;
    }

    // Handle rename
    if (WasInModel && IsInModel && pDir->Name() != OldName)
    {
        const QModelIndex Index = GetIndexForDirectory(pDir);
        emit dataChanged(Index, Index);
    }
    else if (pNewDir != pOldDir)
    {
        // Remove
        if (WasInModel && !IsInModel)
        {
            CheckRemoveDirectory(pDir);
        }
        // Add
        else if (!WasInModel && IsInModel)
        {
            CheckAddDirectory(pDir);
        }
    }
}
