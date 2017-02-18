#include "CWorldTreeModel.h"
#include "CEditorApplication.h"
#include "CWorldEditor.h"
#include "UICommon.h"
#include <Core/GameProject/CGameProject.h>
#include <QIcon>

CWorldTreeModel::CWorldTreeModel(CWorldEditor *pEditor)
{
    connect(gpEdApp, SIGNAL(ActiveProjectChanged(CGameProject*)), this, SLOT(OnProjectChanged(CGameProject*)));
    connect(pEditor, SIGNAL(MapChanged(CWorld*,CGameArea*)), this, SLOT(OnMapChanged()));
}

int CWorldTreeModel::rowCount(const QModelIndex& rkParent) const
{
    if (!rkParent.isValid())            return mWorldList.size();
    else if (IndexIsWorld(rkParent))    return WorldForIndex(rkParent)->NumAreas();
    else                                return 0;
}

int CWorldTreeModel::columnCount(const QModelIndex&) const
{
    return 2;
}

QModelIndex CWorldTreeModel::index(int Row, int Column, const QModelIndex& rkParent) const
{
    if (!hasIndex(Row, Column, rkParent))
        return QModelIndex();

    // World
    if (!rkParent.isValid())
        return createIndex(Row, Column, quint64((Row << 16) | 0xFFFF));

    // Area
    else
        return createIndex(Row, Column, quint64((rkParent.row() << 16) | (Row & 0xFFFF)) );
}

QModelIndex CWorldTreeModel::parent(const QModelIndex& rkChild) const
{
    if (IndexIsWorld(rkChild))
        return QModelIndex();
    else
        return createIndex((rkChild.internalId() >> 16) & 0xFFFF, 0, rkChild.internalId() | 0xFFFF);
}

QVariant CWorldTreeModel::data(const QModelIndex& rkIndex, int Role) const
{
    if (Role == Qt::DisplayRole || Role == Qt::ToolTipRole)
    {
        CWorld *pWorld = WorldForIndex(rkIndex);

        // World
        if (IndexIsWorld(rkIndex))
        {
            QString WorldAssetName = TO_QSTRING( pWorld->Entry()->Name() );
            CStringTable *pWorldNameString = pWorld->WorldName();

            if (rkIndex.column() == 1)
                return WorldAssetName;

            else
            {
                if (pWorldNameString)
                    return TO_QSTRING( pWorldNameString->String("ENGL", 0) );
                else
                    return WorldAssetName;
            }
        }

        // Area
        else
        {
            u32 AreaIndex = AreaIndexForIndex(rkIndex);
            ASSERT(AreaIndex >= 0 && AreaIndex < pWorld->NumAreas());

            CAssetID AreaAssetID = pWorld->AreaResourceID(AreaIndex);
            CResourceEntry *pAreaEntry = pWorld->Entry()->ResourceStore()->FindEntry(AreaAssetID);
            ASSERT(pAreaEntry);

            QString AreaAssetName = TO_QSTRING( pAreaEntry->Name() );
            CStringTable *pAreaNameString = pWorld->AreaName(AreaIndex);

            if (rkIndex.column() == 1)
                return AreaAssetName;

            else
            {
                if (pAreaNameString)
                    return TO_QSTRING( pAreaNameString->String("ENGL", 0) );
                else
                    return "!!" + AreaAssetName;
            }
        }
    }

    else if (Role == Qt::DecorationRole)
    {
        static QIcon sWorldIcon = QIcon(":/icons/World_16px.png");
        static QIcon sAreaIcon  = QIcon(":/icons/New_16px.png");

        if (rkIndex.column() == 1)
            return QVariant::Invalid;
        else if (IndexIsWorld(rkIndex))
            return sWorldIcon;
        else
            return sAreaIcon;
    }

    else if (Role == Qt::FontRole)
    {
        CWorld *pWorld = WorldForIndex(rkIndex);
        ASSERT(pWorld);

        QFont Font;
        int PointSize = Font.pointSize() + 2;

        if (IndexIsWorld(rkIndex))
        {
            PointSize += 1;

            CWorld *pWorld = WorldForIndex(rkIndex);
            if (gpEdApp->WorldEditor()->ActiveWorld() == pWorld)
                Font.setBold(true);
        }
        else
        {
            CResourceEntry *pEntry = AreaEntryForIndex(rkIndex);
            ASSERT(pEntry);

            if (pEntry->IsLoaded())
            {
                if (gpEdApp->WorldEditor()->ActiveArea() == pEntry->Resource())
                    Font.setBold(true);
                else
                    Font.setItalic(true);
            }
        }

        Font.setPointSize(PointSize);
        return Font;
    }

    return QVariant::Invalid;
}

QVariant CWorldTreeModel::headerData(int Section, Qt::Orientation Orientation, int Role) const
{
    if (Orientation == Qt::Horizontal && Role == Qt::DisplayRole)
    {
        if (Section == 0)
            return "In-Game Name";
        else
            return "Internal Name";
    }
    return QVariant::Invalid;
}

bool CWorldTreeModel::IndexIsWorld(const QModelIndex& rkIndex) const
{
    return AreaIndexForIndex(rkIndex) == 0xFFFF;
}

CWorld* CWorldTreeModel::WorldForIndex(const QModelIndex& rkIndex) const
{
    int WorldIndex = (rkIndex.internalId() >> 16) & 0xFFFF;
    return mWorldList[WorldIndex].pWorld;
}

int CWorldTreeModel::AreaIndexForIndex(const QModelIndex& rkIndex) const
{
    int InternalID = (int) rkIndex.internalId();
    return (InternalID & 0xFFFF);
}

CResourceEntry* CWorldTreeModel::AreaEntryForIndex(const QModelIndex& rkIndex) const
{
    ASSERT(rkIndex.isValid() && !IndexIsWorld(rkIndex));
    const SWorldInfo& rkInfo = mWorldList[rkIndex.parent().row()];
    return rkInfo.Areas[rkIndex.row()];
}

// ************ SLOTS ************
void CWorldTreeModel::OnProjectChanged(CGameProject *pProj)
{
    beginResetModel();
    mWorldList.clear();

    if (pProj)
    {
        std::list<CAssetID> WorldIDs;
        pProj->GetWorldList(WorldIDs);
        QList<CAssetID> QWorldIDs = QList<CAssetID>::fromStdList(WorldIDs);

        foreach (const CAssetID& rkID, QWorldIDs)
        {
            CResourceEntry *pEntry = pProj->ResourceStore()->FindEntry(rkID);

            if (pEntry)
            {
                TResPtr<CWorld> pWorld = pEntry->Load();

                if (pWorld)
                {
                    SWorldInfo Info;
                    Info.pWorld = pWorld;

                    for (u32 iArea = 0; iArea < pWorld->NumAreas(); iArea++)
                    {
                        CAssetID AreaID = pWorld->AreaResourceID(iArea);
                        CResourceEntry *pAreaEntry = pWorld->Entry()->ResourceStore()->FindEntry(AreaID);
                        ASSERT(pAreaEntry);
                        Info.Areas << pAreaEntry;
                    }

                    mWorldList << Info;
                }
            }
        }
    }

    endResetModel();
}

void CWorldTreeModel::OnMapChanged()
{
    // Flag all data as changed to ensure the font updates correctly based on which areas are loaded
    // note we don't know which areas used to be loaded, so flagging those specific indices isn't an option
    int MaxRow = rowCount(QModelIndex()) - 1;
    int MaxCol = columnCount(QModelIndex()) - 1;
    emit dataChanged(index(0, 0, QModelIndex()), index(MaxRow, MaxCol, QModelIndex()));
}

// ************ PROXY MODEL ************
bool CWorldTreeProxyModel::lessThan(const QModelIndex& rkSourceLeft, const QModelIndex& rkSourceRight) const
{
    CWorldTreeModel *pModel = qobject_cast<CWorldTreeModel*>(sourceModel());
    ASSERT(pModel != nullptr);

    if (pModel->IndexIsWorld(rkSourceLeft))
    {
        ASSERT(pModel->IndexIsWorld(rkSourceRight));
        bool IsLessThan = (rkSourceLeft.row() < rkSourceRight.row());
        return (sortOrder() == Qt::AscendingOrder ? IsLessThan : !IsLessThan);
    }
    else
        return pModel->data(rkSourceLeft, Qt::DisplayRole).toString().toUpper() < pModel->data(rkSourceRight, Qt::DisplayRole).toString().toUpper();
}

bool CWorldTreeProxyModel::filterAcceptsRow(int SourceRow, const QModelIndex& rkSourceParent) const
{
    // Always accept worlds
    if (!rkSourceParent.isValid() || mFilterString.isEmpty())
        return true;

    CWorldTreeModel *pModel = qobject_cast<CWorldTreeModel*>(sourceModel());
    ASSERT(pModel != nullptr);

    for (int iCol = 0; iCol < pModel->columnCount(rkSourceParent); iCol++)
    {
        QModelIndex Index = pModel->index(SourceRow, iCol, rkSourceParent);
        if (pModel->data(Index, Qt::DisplayRole).toString().contains(mFilterString, Qt::CaseInsensitive))
            return true;
    }

    return false;
}
