#include "Editor/ResourceBrowser/CVirtualDirectoryModel.h"

#include "Editor/UICommon.h"
#include "Editor/ResourceBrowser/CResourceBrowser.h"
#include "Editor/ResourceBrowser/CResourceMimeData.h"
#include <Core/GameProject/CVirtualDirectory.h>

#include <algorithm>
#include <QIcon>

CVirtualDirectoryModel::CVirtualDirectoryModel(CResourceBrowser *pBrowser, QObject *pParent)
    : QAbstractItemModel(pParent)
{
    connect(pBrowser, &CResourceBrowser::DirectoryAboutToBeMoved, this, &CVirtualDirectoryModel::OnDirectoryAboutToBeMoved);
    connect(pBrowser, &CResourceBrowser::DirectoryMoved, this, &CVirtualDirectoryModel::FinishModelChanges);

    connect(pBrowser, &CResourceBrowser::DirectoryAboutToBeCreated, this, &CVirtualDirectoryModel::OnDirectoryAboutToBeCreated);
    connect(pBrowser, &CResourceBrowser::DirectoryCreated, this, &CVirtualDirectoryModel::FinishModelChanges);

    connect(pBrowser, &CResourceBrowser::DirectoryAboutToBeDeleted, this, &CVirtualDirectoryModel::OnDirectoryAboutToBeDeleted);
    connect(pBrowser, &CResourceBrowser::DirectoryDeleted, this, &CVirtualDirectoryModel::FinishModelChanges);
}

CVirtualDirectoryModel::~CVirtualDirectoryModel() = default;

QModelIndex CVirtualDirectoryModel::index(int Row, int Column, const QModelIndex& rkParent) const
{
    if (!hasIndex(Row, Column, rkParent))
        return {};

    CVirtualDirectory *pDir = IndexDirectory(rkParent);

    if (pDir != nullptr && pDir->NumSubdirectories() > static_cast<uint32_t>(Row))
        return createIndex(Row, Column, pDir->SubdirectoryByIndex(Row));

    if (pDir == nullptr)
        return createIndex(Row, Column, mpRoot);

    return {};
}

QModelIndex CVirtualDirectoryModel::parent(const QModelIndex& rkChild) const
{
    CVirtualDirectory *pDir = IndexDirectory(rkChild);

    if (CVirtualDirectory* pParent = pDir->Parent())
    {
        if (CVirtualDirectory* pGrandparent = pParent->Parent())
        {
            for (size_t iSub = 0; iSub < pGrandparent->NumSubdirectories(); iSub++)
            {
                if (pGrandparent->SubdirectoryByIndex(iSub) == pParent)
                    return createIndex(static_cast<int>(iSub), 0, pParent);
            }
        }
        else
        {
            return createIndex(0, 0, mpRoot);
        }
    }

    return {};
}

int CVirtualDirectoryModel::rowCount(const QModelIndex& rkParent) const
{
    if (const CVirtualDirectory* pDir = IndexDirectory(rkParent))
        return static_cast<int>(pDir->NumSubdirectories());

    return mpRoot ? 1 : 0;
}

int CVirtualDirectoryModel::columnCount(const QModelIndex&) const
{
    return 1;
}

QVariant CVirtualDirectoryModel::data(const QModelIndex& rkIndex, int Role) const
{
    if (!rkIndex.isValid())
        return {};

    if (Role == Qt::DisplayRole || Role == Qt::ToolTipRole)
    {
        if (!rkIndex.parent().isValid())
        {
            return tr("Resources");
        }
        else
        {
            if (const auto* pDir = IndexDirectory(rkIndex))
                return TO_QSTRING(pDir->Name());
        }
    }

    if (Role == Qt::DecorationRole)
    {
        return QIcon(QStringLiteral(":/icons/Open_24px.svg"));
    }

    return {};
}

bool CVirtualDirectoryModel::setData(const QModelIndex& rkIndex, const QVariant& rkValue, int Role)
{
    if (Role == Qt::EditRole)
    {
        const QString NewName = rkValue.toString();

        if (auto* pDir = IndexDirectory(rkIndex))
        {
            gpEdApp->ResourceBrowser()->RenameDirectory(pDir, TO_TSTRING(NewName));
            return true;
        }
    }

    return false;
}

Qt::ItemFlags CVirtualDirectoryModel::flags(const QModelIndex& rkIndex) const
{
    Qt::ItemFlags Out = Qt::ItemIsSelectable | Qt::ItemIsDropEnabled | Qt::ItemIsEnabled;

    if (rkIndex.isValid() && rkIndex.parent().isValid())
        Out |= Qt::ItemIsEditable | Qt::ItemIsDragEnabled;

    return Out;
}

bool CVirtualDirectoryModel::canDropMimeData(const QMimeData *pkData, Qt::DropAction Action, int Row, int Column, const QModelIndex& rkParent) const
{
    // Don't allow dropping between items
    if (Row != -1 || Column != -1)
        return false;

    if (Action == Qt::MoveAction)
    {
        const auto* pDir = IndexDirectory(rkParent);
        if (!pDir)
            return false;

        const auto* pkMimeData = qobject_cast<const CResourceMimeData*>(pkData);
        if (!pkMimeData)
            return false;

        // Don't allow moving a directory into one of its children
        const auto& dirs = pkMimeData->Directories();
        return std::ranges::none_of(dirs, [pDir](const auto* moveDir) { return pDir->IsDescendantOf(moveDir); });
    }

    return false;
}

bool CVirtualDirectoryModel::dropMimeData(const QMimeData *pkData, Qt::DropAction Action, int Row, int Column, const QModelIndex& rkParent)
{
    // Perform move!
    const auto* pkMimeData = qobject_cast<const CResourceMimeData*>(pkData);

    if (canDropMimeData(pkData, Action, Row, Column, rkParent))
    {
        CVirtualDirectory *pDir = IndexDirectory(rkParent);
        ASSERT(pDir);

        return gpEdApp->ResourceBrowser()->MoveResources(pkMimeData->Resources(), pkMimeData->Directories(), pDir);
    }

    return false;
}

QMimeData* CVirtualDirectoryModel::mimeData(const QModelIndexList& rkIndexes) const
{
    if (rkIndexes.size() != 1)
        return nullptr;

    const QModelIndex Index = rkIndexes.front();
    CVirtualDirectory *pDir = IndexDirectory(Index);
    return new CResourceMimeData(pDir);
}

Qt::DropActions CVirtualDirectoryModel::supportedDragActions() const
{
    return Qt::CopyAction | Qt::MoveAction;
}

Qt::DropActions CVirtualDirectoryModel::supportedDropActions() const
{
    return Qt::MoveAction;
}

QModelIndex CVirtualDirectoryModel::GetIndexForDirectory(const CVirtualDirectory *pDir) const
{
    if (pDir == nullptr)
        return {};

    QList<int> Indices;
    const auto* pOriginal = pDir;
    const auto* pParent = pDir->Parent();

    // Get index list
    while (pParent != nullptr)
    {
        [[maybe_unused]] bool Found = false;

        for (size_t iDir = 0; iDir < pParent->NumSubdirectories(); iDir++)
        {
            if (pParent->SubdirectoryByIndex(iDir) == pDir)
            {
                Indices.push_front(static_cast<int>(iDir));
                pDir = pParent;
                pParent = pParent->Parent();
                Found = true;
                break;
            }
        }

        ASSERT(Found); // it should not be possible for this not to work
    }

    // Traverse hierarchy
    QModelIndex Out = index(0, 0);

    for (const int Idx : Indices)
        Out = index(Idx, 0, Out);

    ASSERT(IndexDirectory(Out) == pOriginal);
    return Out;
}

CVirtualDirectory* CVirtualDirectoryModel::IndexDirectory(const QModelIndex& rkIndex) const
{
    if (!rkIndex.isValid())
        return nullptr;

    return static_cast<CVirtualDirectory*>(rkIndex.internalPointer());
}

void CVirtualDirectoryModel::SetRoot(CVirtualDirectory *pDir)
{
    beginResetModel();
    mpRoot = pDir;
    endResetModel();
}

std::optional<std::pair<QModelIndex, int>> CVirtualDirectoryModel::GetProposedIndex(const QString& Path) const
{
    // Get parent path
    TString FullPath = TO_TSTRING(Path);

    if (FullPath.EndsWith('/') || FullPath.EndsWith('\\'))
        FullPath = FullPath.ChopBack(1);

    const uint64_t LastSlash = FullPath.LastIndexOf("\\/");
    const TString ParentPath = FullPath.ChopBack(FullPath.Size() - LastSlash);

    // Find parent index
    const CVirtualDirectory* pParent = (ParentPath.IsEmpty() ? mpRoot : mpRoot->FindChildDirectory(ParentPath, false));
    if (pParent == nullptr)
        return std::nullopt;

    const QModelIndex ParentIndex = GetIndexForDirectory(pParent);
    if (!ParentIndex.isValid())
        return std::nullopt;

    // Determine the row number that the new directory will be inserted at
    const QString DirName = TO_QSTRING(FullPath.ChopFront(LastSlash + 1));
    const int NumRows = rowCount(ParentIndex);
    int RowIdx = 0;

    for (; RowIdx < NumRows; RowIdx++)
    {
        const QModelIndex Index = index(RowIdx, 0, ParentIndex);
        const QString OtherName = data(Index, Qt::DisplayRole).toString();

        if (QString::compare(DirName, OtherName, Qt::CaseInsensitive) < 0)
            break;
    }

    return std::make_pair(ParentIndex, RowIdx);
}

void CVirtualDirectoryModel::OnDirectoryAboutToBeMoved(const CVirtualDirectory *pDir, const QString& NewPath)
{
    const auto indexOptional = GetProposedIndex(NewPath);

    if (!indexOptional)
        return;

    const auto [Parent, Row] = *indexOptional;
    const QModelIndex OldIndex = GetIndexForDirectory(pDir);
    const QModelIndex OldParent = OldIndex.parent();
    const int OldRow = OldIndex.row();

    if (OldParent == Parent && (Row == OldRow || Row == OldRow + 1))
    {
        emit layoutAboutToBeChanged();
        mChangingLayout = true;
    }
    else
    {
        beginMoveRows(OldParent, OldRow, OldRow, Parent, Row);
        mMovingRows = true;
    }
}

void CVirtualDirectoryModel::OnDirectoryAboutToBeCreated(const QString& DirPath)
{
    const auto indexOptional = GetProposedIndex(DirPath);

    if (!indexOptional)
        return;

    const auto [Parent, Row] = *indexOptional;
    beginInsertRows(Parent, Row, Row);
    mInsertingRows = true;
}

void CVirtualDirectoryModel::OnDirectoryAboutToBeDeleted(const CVirtualDirectory *pDir)
{
    const QModelIndex Index = GetIndexForDirectory(pDir);

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
    if (mMovingRows)
    {
        endMoveRows();
        mMovingRows = false;
    }
    if (mChangingLayout)
    {
        emit layoutChanged();
        mChangingLayout = false;
    }
}
