#ifndef CRESOURCETABLEMODEL
#define CRESOURCETABLEMODEL

#include "Editor/UICommon.h"
#include <Core/GameProject/CResourceEntry.h>
#include <Core/GameProject/CResourceIterator.h>
#include <Core/GameProject/CResourceStore.h>
#include <Core/Resource/CResource.h>
#include <QAbstractTableModel>
#include <QIcon>

class CResourceTableModel : public QAbstractTableModel
{
    Q_OBJECT

    QList<CVirtualDirectory*> mDirectories;
    QList<CResourceEntry*> mEntries;
    QMap<CResourceEntry*, int> mEntryIndexMap;
    bool mHasParent;

public:
    CResourceTableModel(QObject *pParent = 0)
        : QAbstractTableModel(pParent)
    {}

    int rowCount(const QModelIndex& /*rkParent*/) const
    {
        return mDirectories.size() + mEntries.size();
    }

    int columnCount(const QModelIndex& /*rkParent*/) const
    {
        return 1;
    }

    QVariant data(const QModelIndex& rkIndex, int Role) const
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

    QModelIndex GetIndexForEntry(CResourceEntry *pEntry) const
    {
        if (mEntryIndexMap.contains(pEntry))
            return index(mEntryIndexMap[pEntry] + mDirectories.size(), 0, QModelIndex());
        else
            return QModelIndex();
    }

    CResourceEntry* IndexEntry(const QModelIndex& rkIndex) const
    {
        int Index = rkIndex.row() - mDirectories.size();
        return (Index >= 0 ? mEntries[Index] : nullptr);
    }

    CVirtualDirectory* IndexDirectory(const QModelIndex& rkIndex) const
    {
        return (rkIndex.row() < mDirectories.size() ? mDirectories[rkIndex.row()] : nullptr);
    }

    bool IsIndexDirectory(const QModelIndex& rkIndex) const
    {
        return rkIndex.row() < mDirectories.size();
    }

    void FillEntryList(CVirtualDirectory *pDir, bool AssetListMode)
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

protected:
    void RecursiveAddDirectoryContents(CVirtualDirectory *pDir)
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

public:
    // Accessors
    inline u32 NumDirectories() const   { return mDirectories.size(); }
    inline u32 NumResources() const     { return mEntries.size(); }
};

#endif // CRESOURCELISTMODEL

