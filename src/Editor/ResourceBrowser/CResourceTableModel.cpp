#include "Editor/ResourceBrowser/CResourceTableModel.h"

#include "Editor/UICommon.h"
#include "Editor/ResourceBrowser/CResourceBrowser.h"
#include "Editor/ResourceBrowser/CResourceMimeData.h"
#include <Core/GameProject/CResourceEntry.h>
#include <Common/TString.h>

#include <algorithm>

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

CResourceTableModel::~CResourceTableModel() = default;

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
        return {};

    // Directory
    if (IsIndexDirectory(rkIndex))
    {
        const auto* pDir = IndexDirectory(rkIndex);

        if (Role == Qt::DisplayRole || Role == Qt::ToolTipRole)
        {
            return ((mpCurrentDir && !mpCurrentDir->IsRoot() && rkIndex.row() == 0)
                        ? QStringLiteral("..")
                        : TO_QSTRING(pDir->Name()));
        }

        if (Role == Qt::DecorationRole)
            return QIcon(QStringLiteral(":/icons/Open_24px.svg"));

        return {};
    }

    // Resource
    const auto* pEntry = IndexEntry(rkIndex);

    if (Role == Qt::DisplayRole)
        return TO_QSTRING(pEntry->Name());

    if (Role == Qt::ToolTipRole)
        return TO_QSTRING(pEntry->CookedAssetPath(true));

    if (Role == Qt::DecorationRole)
        return QIcon(QStringLiteral(":/icons/Sphere Preview.svg"));

    return {};
}

Qt::ItemFlags CResourceTableModel::flags(const QModelIndex& rkIndex) const
{
    Qt::ItemFlags Out = Qt::ItemIsSelectable |Qt::ItemIsEnabled;
    const auto* pDir = IndexDirectory(rkIndex);

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

    if (const auto* pkMimeData = qobject_cast<const CResourceMimeData*>(pkData))
    {
        // Don't allow dropping onto blank space in asset list mode
        if (!rkParent.isValid() && mIsAssetListMode)
            return false;

        // Make sure we're dropping onto a directory
        if (const auto* pDir = (rkParent.isValid() ? IndexDirectory(rkParent) : mpCurrentDir))
        {
            // Make sure this directory isn't part of the mime data, or a subdirectory of a directory in the mime data
            for (const auto* pMimeDir : pkMimeData->Directories())
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
    const auto* pkMimeData = qobject_cast<const CResourceMimeData*>(pkData);

    if (canDropMimeData(pkData, Action, Row, Column, rkParent))
    {
        auto* pDir = (rkParent.isValid() ? IndexDirectory(rkParent) : mpCurrentDir);
        ASSERT(pDir);

        return gpEdApp->ResourceBrowser()->MoveResources(pkMimeData->Resources(), pkMimeData->Directories(), pDir);
    }
    return false;
}

QMimeData* CResourceTableModel::mimeData(const QModelIndexList& rkIndexes) const
{
    if (rkIndexes.isEmpty())
        return nullptr;

    QList<CResourceEntry*> Resources;
    QList<CVirtualDirectory*> Dirs;

    for (const QModelIndex& Index : rkIndexes)
    {
        auto* pEntry = IndexEntry(Index);
        auto* pDir = IndexDirectory(Index);

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
QModelIndex CResourceTableModel::GetIndexForEntry(const CResourceEntry *pEntry) const
{
    const auto iter = std::ranges::find(mEntries, pEntry);
    if (iter == mEntries.cend())
        return {};

    const auto idx = std::distance(mEntries.begin(), iter);
    return index(int(mDirectories.size() + idx), 0);
}

QModelIndex CResourceTableModel::GetIndexForDirectory(const CVirtualDirectory *pDir) const
{
    const auto iter = std::ranges::find(mDirectories, pDir);
    if (iter == mDirectories.end())
        return {};

    const auto Index = std::distance(mDirectories.begin(), iter);
    return index(int(Index), 0);
}

CResourceEntry* CResourceTableModel::IndexEntry(const QModelIndex& rkIndex) const
{
    const auto Index = rkIndex.row() - mDirectories.size();
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
                auto* pEntry = pDir->ResourceByIndex(iRes);

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

void CResourceTableModel::DisplayEntryList(QList<CResourceEntry*> rkEntries, QString rkListDescription)
{
    beginResetModel();
    mEntries = std::move(rkEntries);
    mDirectories.clear();
    mModelDescription = std::move(rkListDescription);
    mIsAssetListMode = true;
    mIsDisplayingUserEntryList = true;
    endResetModel();
}

void CResourceTableModel::RecursiveAddDirectoryContents(CVirtualDirectory *pDir)
{
    for (size_t iRes = 0; iRes < pDir->NumResources(); iRes++)
    {
        auto* pEntry = pDir->ResourceByIndex(iRes);

        if (!pEntry->IsHidden())
        {
            const int Index = EntryListIndex(pEntry);
            mEntries.insert(Index, pEntry);
        }
    }

    for (size_t iDir = 0; iDir < pDir->NumSubdirectories(); iDir++)
        RecursiveAddDirectoryContents(pDir->SubdirectoryByIndex(iDir));
}

int CResourceTableModel::EntryListIndex(const CResourceEntry *pEntry) const
{
    return int(std::ranges::lower_bound(mEntries, pEntry) - mEntries.cbegin());
}

void CResourceTableModel::RefreshAllIndices()
{
    const int NumRows = rowCount();
    const int NumCols = columnCount();

    if (NumRows > 0 && NumCols > 0)
    {
        emit dataChanged(index(0, 0), index(NumRows - 1, NumCols - 1));
    }
}

void CResourceTableModel::CheckAddResource(CResourceEntry *pEntry)
{
    if ((mIsAssetListMode && pEntry->IsInDirectory(mpCurrentDir)) ||
        (!mIsAssetListMode && pEntry->Directory() == mpCurrentDir))
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
    const auto Index = mEntries.indexOf(pEntry);
    if (Index == -1)
        return;

    const auto RowIndex = int(Index + mDirectories.size());

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
    const auto Index = int(mDirectories.indexOf(pDir));

    if (Index == -1)
        return;

    beginRemoveRows(QModelIndex(), Index, Index);
    mDirectories.removeAt(Index);
    endRemoveRows();
}

void CResourceTableModel::OnResourceMoved(CResourceEntry *pEntry, CVirtualDirectory *pOldDir, const TString& OldName)
{
    const auto* pNewDir = pEntry->Directory();
    const bool WasInModel = (pOldDir == mpCurrentDir || (mIsAssetListMode && pOldDir->IsDescendantOf(mpCurrentDir)));
    const bool IsInModel = (pNewDir == mpCurrentDir || (mIsAssetListMode && pNewDir->IsDescendantOf(mpCurrentDir)));

    // Handle rename
    if (WasInModel && IsInModel && pEntry->Name() != OldName)
    {
        const int ResIdx = EntryListIndex(pEntry);
        const int Row = ResIdx + mDirectories.size();
        const QModelIndex Index = index(Row, 0);
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

void CResourceTableModel::OnDirectoryMoved(CVirtualDirectory *pDir, CVirtualDirectory *pOldDir, const TString& OldName)
{
    auto* pNewDir = pDir->Parent();
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
