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
        return 3;
    }

    QVariant data(const QModelIndex& rkIndex, int Role) const
    {
        u32 Col = rkIndex.column();

        // Directory
        if (IsIndexDirectory(rkIndex))
        {
            if (Col != 0)
                return QVariant::Invalid;

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
        {
            if (Col == 0)
            {
                return TO_QSTRING(pEntry->Name());
            }

            if (Col == 1)
            {
                return TO_QSTRING(pEntry->TypeInfo()->TypeName());
            }

            if (Col == 2)
            {
                u64 Size = pEntry->Size();
                return Size ? TO_QSTRING( TString::FileSizeString(pEntry->Size()) ) : "";
            }
        }

        else if (Role == Qt::ToolTipRole)
            return TO_QSTRING(pEntry->CookedAssetPath(true));

        else if (Role == Qt::TextAlignmentRole && rkIndex.column() == 2)
            return Qt::AlignRight;

        return QVariant::Invalid;
    }

    QModelIndex GetIndexForEntry(CResourceEntry *pEntry) const
    {
        for (int iRes = 0; iRes < mEntries.size(); iRes++)
        {
            if (mEntries[iRes] == pEntry)
            {
                QModelIndex Out = index(mDirectories.size() + iRes, 0);
                ASSERT(IndexEntry(Out) == pEntry);
                return Out;
            }
        }

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

    void FillEntryList(CVirtualDirectory *pDir, bool IsSearching)
    {
        beginResetModel();

        mEntries.clear();
        mDirectories.clear();
        mHasParent = false;

        if (pDir)
        {
            // When not searching, show only subdirectories and assets in the current directory.
            if (!IsSearching)
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
                        mEntries << pEntry;
                }
            }

            // When searching, do not show subdirectories and show all assets in current directory + all subdirectories.
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
                mEntries << pDir->ResourceByIndex(iRes);
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

