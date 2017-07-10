#ifndef CFILTEREDRESOURCEMODEL_H
#define CFILTEREDRESOURCEMODEL_H

#include "CResourceSelector.h"
#include "Editor/UICommon.h"

#include <Core/GameProject/CResourceEntry.h>
#include <Core/GameProject/CResourceIterator.h>
#include <Core/Resource/CResTypeFilter.h>

#include <QAbstractTableModel>
#include <QSortFilterProxyModel>

class CFilteredResourceModel : public QAbstractTableModel
{
    Q_OBJECT
    QVector<CResourceEntry*> mEntries;
    int mInitialRow;

public:
    CFilteredResourceModel(CResourceSelector *pSelector, QObject *pParent = 0)
        : QAbstractTableModel(pParent)
        , mInitialRow(0)
    {
        const CResTypeFilter& rkFilter = pSelector->TypeFilter();

        for (CResourceIterator It; It; ++It)
        {
            if (rkFilter.Accepts(*It))
            {
                mEntries << *It;
            }
        }

        qSort(mEntries.begin(), mEntries.end(), [](CResourceEntry *pA, CResourceEntry *pB) -> bool {
            return pA->UppercaseName() < pB->UppercaseName();
        });

        for (int ResIdx = 0; ResIdx < mEntries.size(); ResIdx++)
        {
            if (mEntries[ResIdx] == pSelector->Entry())
            {
                mInitialRow = ResIdx;
                break;
            }
        }
    }

    // QAbstractTableModel interface
    int rowCount(const QModelIndex&) const
    {
        return mEntries.size();
    }

    int columnCount(const QModelIndex&) const
    {
        return 1;
    }

    QVariant data(const QModelIndex& rkIndex, int Role) const
    {
        CResourceEntry *pEntry = EntryForIndex(rkIndex);

        if (rkIndex.column() == 0)
        {
            if (Role == Qt::DisplayRole)
            {
                return TO_QSTRING( pEntry->Name() + "." + pEntry->CookedExtension() );
            }
            else if (Role == Qt::ToolTipRole)
            {
                return TO_QSTRING( pEntry->CookedAssetPath(true) );
            }
            else if (Role == Qt::DecorationRole)
            {
                return QIcon(":/icons/Sphere Preview.png");
            }
        }
        else
        {
            if (Role == Qt::DisplayRole || Role == Qt::ToolTipRole)
            {
                return TO_QSTRING( pEntry->TypeInfo()->TypeName() );
            }
        }

        return QVariant::Invalid;
    }

    // Accessors
    inline QModelIndex InitialIndex() const
    {
        return index(mInitialRow, 0);
    }

    inline CResourceEntry* EntryForIndex(const QModelIndex& rkIndex) const
    {
        return mEntries[rkIndex.row()];
    }
};

class CFilteredResourceProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
    TString mSearchString;

public:
    CFilteredResourceProxyModel(QObject *pParent = 0)
        : QSortFilterProxyModel(pParent)
    {}

    bool filterAcceptsRow(int SourceRow, const QModelIndex&) const
    {
        if (mSearchString.IsEmpty())
            return true;

        CFilteredResourceModel *pModel = qobject_cast<CFilteredResourceModel*>(sourceModel());
        ASSERT(pModel);

        QModelIndex SrcIndex = pModel->index(SourceRow, 0);
        return pModel->EntryForIndex(SrcIndex)->UppercaseName().Contains(mSearchString);
    }

    inline void SetSearchString(const QString& rkString)
    {
        mSearchString = TO_TSTRING(rkString).ToUpper();
        invalidate();
    }
};

#endif // CFILTEREDRESOURCEMODEL_H
