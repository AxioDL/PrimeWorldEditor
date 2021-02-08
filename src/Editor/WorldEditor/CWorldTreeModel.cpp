#include "CWorldTreeModel.h"
#include "CEditorApplication.h"
#include "CWorldEditor.h"
#include "UICommon.h"
#include <Core/GameProject/CGameProject.h>
#include <Core/GameProject/CResourceIterator.h>
#include <QIcon>

#include <memory>

CWorldTreeModel::CWorldTreeModel(CWorldEditor *pEditor)
{
    connect(gpEdApp, &CEditorApplication::ActiveProjectChanged, this, &CWorldTreeModel::OnProjectChanged);
    connect(pEditor, &CWorldEditor::MapChanged, this, &CWorldTreeModel::OnMapChanged);
}

int CWorldTreeModel::rowCount(const QModelIndex& rkParent) const
{
    if (!rkParent.isValid())
        return mWorldList.size();

    if (IndexIsWorld(rkParent))
        return mWorldList[rkParent.row()].Areas.size();

    return 0;
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
    return createIndex(Row, Column, quint64((rkParent.row() << 16) | (Row & 0xFFFF)) );
}

QModelIndex CWorldTreeModel::parent(const QModelIndex& rkChild) const
{
    if (IndexIsWorld(rkChild))
        return QModelIndex();

    return createIndex((rkChild.internalId() >> 16) & 0xFFFF, 0, rkChild.internalId() | 0xFFFF);
}

QVariant CWorldTreeModel::data(const QModelIndex& rkIndex, int Role) const
{
    if (Role == Qt::DisplayRole || Role == Qt::ToolTipRole)
    {
        const SWorldInfo& rkInfo = WorldInfoForIndex(rkIndex);

        // World
        if (IndexIsWorld(rkIndex))
        {
            // For Corruption worlds, we swap the columns around. This is because Corruption's in-game world names
            // are often missing, confusing, or just straight-up inaccurate, which makes the internal name a better
            // means of telling worlds apart.
            // For DKCR worlds, we only display the world name in the first column.
            const int InternalNameCol = (gpEdApp->ActiveProject()->Game() >= EGame::Corruption ? 0 : 1);

            // Internal name
            if (rkIndex.column() == InternalNameCol)
                return rkInfo.WorldName;

            // In-Game name
            if (rkInfo.pWorld != nullptr)
                return TO_QSTRING(rkInfo.pWorld->InGameName());

            return QString{};
        }
        else // Area
        {
            const CWorld *pWorld = WorldForIndex(rkIndex);
            const int AreaIndex = AreaIndexForIndex(rkIndex);
            ASSERT(pWorld);

            const TString AreaInternalName = pWorld->AreaInternalName(AreaIndex);
            const TString AreaInGameName = (gpEdApp->ActiveProject()->Game() == EGame::DKCReturns ? pWorld->InGameName() : pWorld->AreaInGameName(AreaIndexForIndex(rkIndex)));

            // Return name
            if (rkIndex.column() == 1)
                return TO_QSTRING(AreaInternalName);
            else
                return TO_QSTRING(AreaInGameName);
        }
    }

    if (Role == Qt::DecorationRole)
    {
        static const QIcon sWorldIcon = QIcon(QStringLiteral(":/icons/World_16px.svg"));
        static const QIcon sAreaIcon  = QIcon(QStringLiteral(":/icons/New_16px.svg"));

        if (rkIndex.column() == 1)
            return QVariant::Invalid;
        else if (IndexIsWorld(rkIndex))
            return sWorldIcon;
        else
            return sAreaIcon;
    }

    if (Role == Qt::FontRole)
    {
        QFont Font;
        int PointSize = Font.pointSize() + 2;

        if (IndexIsWorld(rkIndex))
        {
            PointSize += 1;

            const SWorldInfo& rkInfo = WorldInfoForIndex(rkIndex);

            if (CWorld* pActiveWorld = gpEdApp->WorldEditor()->ActiveWorld())
            {
                const EGame Game = gpEdApp->ActiveProject()->Game();
                const bool IsActiveWorld = (Game <= EGame::Corruption && rkInfo.pWorld == pActiveWorld) ||
                                           (Game == EGame::DKCReturns && rkInfo.Areas.contains(pActiveWorld->Entry()));

                if (IsActiveWorld)
                    Font.setBold(true);
            }
        }
        else
        {
            const CResourceEntry *pEntry = AreaEntryForIndex(rkIndex);

            if (pEntry != nullptr && pEntry->IsLoaded())
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
            return tr("In-Game Name");
        else
            return tr("Internal Name");
    }
    return QVariant::Invalid;
}

bool CWorldTreeModel::IndexIsWorld(const QModelIndex& rkIndex) const
{
    const auto AreaIndex = static_cast<int>(rkIndex.internalId()) & 0xFFFF;
    return AreaIndex == 0xFFFF;
}

int CWorldTreeModel::AreaIndexForIndex(const QModelIndex& rkIndex) const
{
    if (gpEdApp->ActiveProject()->Game() == EGame::DKCReturns)
        return 0;

    const auto InternalID = static_cast<int>(rkIndex.internalId());
    return InternalID & 0xFFFF;
}

CWorld* CWorldTreeModel::WorldForIndex(const QModelIndex& rkIndex) const
{
    ASSERT(rkIndex.isValid());
    const SWorldInfo& rkInfo = WorldInfoForIndex(rkIndex);

    if (gpEdApp->ActiveProject()->Game() == EGame::DKCReturns && !IndexIsWorld(rkIndex))
    {
        const auto AreaIndex = static_cast<int>(rkIndex.internalId() & 0xFFFF);
        CResourceEntry *pEntry = rkInfo.Areas[AreaIndex];
        return pEntry != nullptr ? static_cast<CWorld*>(pEntry->Load()) : nullptr;
    }

    return rkInfo.pWorld;
}

CResourceEntry* CWorldTreeModel::AreaEntryForIndex(const QModelIndex& rkIndex) const
{
    ASSERT(rkIndex.isValid() && !IndexIsWorld(rkIndex));

    CAssetID AreaID;
    if (const CWorld* pWorld = WorldForIndex(rkIndex))
    {
        const int AreaIndex = AreaIndexForIndex(rkIndex);
        AreaID = pWorld->AreaResourceID(AreaIndex);
    }

    return gpResourceStore->FindEntry(AreaID);
}

const CWorldTreeModel::SWorldInfo& CWorldTreeModel::WorldInfoForIndex(const QModelIndex& rkIndex) const
{
    const int WorldIndex = (static_cast<int>(rkIndex.internalId()) >> 16) & 0xFFFF;
    return mWorldList[WorldIndex];
}

// ************ SLOTS ************
void CWorldTreeModel::OnProjectChanged(CGameProject *pProj)
{
    beginResetModel();
    mWorldList.clear();

    if (pProj != nullptr)
    {
        if (pProj->Game() != EGame::DKCReturns)
        {
            // Metroid Prime series; fetch all world assets
            std::list<CAssetID> WorldIDs;
            pProj->GetWorldList(WorldIDs);
            QList<CAssetID> QWorldIDs = QList<CAssetID>(WorldIDs.begin(), WorldIDs.end());

            for (const CAssetID& rkID : QWorldIDs)
            {
                if (CResourceEntry* pEntry = pProj->ResourceStore()->FindEntry(rkID))
                {
                    TResPtr<CWorld> pWorld = pEntry->Load();

                    if (pWorld != nullptr)
                    {
                        SWorldInfo Info;
                        Info.WorldName = TO_QSTRING( pWorld->Name() );
                        Info.pWorld = pWorld;

                        // Add areas
                        for (size_t iArea = 0; iArea < pWorld->NumAreas(); iArea++)
                        {
                            CAssetID AreaID = pWorld->AreaResourceID(iArea);
                            CResourceEntry *pAreaEntry = pWorld->Entry()->ResourceStore()->FindEntry(AreaID);
                            ASSERT(pAreaEntry);
                            Info.Areas.push_back(pAreaEntry);
                        }

                        mWorldList.push_back(Info);
                    }
                }
            }

            // Sort in alphabetical order for MP3
            if (pProj->Game() >= EGame::Corruption)
            {
                std::sort(mWorldList.begin(), mWorldList.end(), [](const SWorldInfo& rkA, const SWorldInfo& rkB) -> bool {
                    return (rkA.WorldName.toUpper() < rkB.WorldName.toUpper());
                });
            }
        }
        else // DKCR - Get worlds from areas.lst
        {
            TString AreaListPath = pProj->DiscFilesystemRoot(false) + "areas.lst";

            // I really need a good text stream class at some point
            using FILEPtr = std::unique_ptr<FILE, decltype(&std::fclose)>;
            FILEPtr pAreaList{std::fopen(*AreaListPath, "r"), std::fclose};
            SWorldInfo *pInfo = nullptr;
            std::set<CAssetID> UsedWorlds;

            while (!std::feof(pAreaList.get()))
            {
                char LineBuffer[256] = {};
                std::fgets(LineBuffer, sizeof(LineBuffer), pAreaList.get());
                const TString Line(LineBuffer);

                CAssetID WorldID;
                TString WorldName;
                uint32 IDSplit = Line.IndexOf(' ');

                if (IDSplit != -1)
                {
                    // Get world ID
                    const TString IDString = (IDSplit == -1 ? "" : Line.SubString(2, IDSplit - 2));
                    WorldID = CAssetID::FromString(IDString);

                    // Get world name
                    const TString WorldPath = (IDSplit == -1 ? "" : Line.SubString(IDSplit + 1, Line.Size() - IDSplit - 1));
                    const uint32 UnderscoreIdx = WorldPath.IndexOf('_');
                    const uint32 WorldDirEnd = WorldPath.IndexOf("\\/", UnderscoreIdx);

                    if (UnderscoreIdx != -1 && WorldDirEnd != -1)
                        WorldName = WorldPath.SubString(UnderscoreIdx + 1, WorldDirEnd - UnderscoreIdx - 1);
                }

                if (WorldID.IsValid() && !WorldName.IsEmpty())
                {
                    if (CResourceEntry* pEntry = gpResourceStore->FindEntry(WorldID))
                    {
                        QString WorldNameQ = TO_QSTRING(WorldName);

                        if (!pInfo || pInfo->WorldName != WorldNameQ)
                        {
                            mWorldList.push_back(SWorldInfo());
                            pInfo = &mWorldList.back();
                            pInfo->WorldName = WorldNameQ;
                        }

                        pInfo->Areas.push_back(pEntry);
                        UsedWorlds.insert(pEntry->ID());
                    }
                }
            }
            pAreaList.reset();

            // Add remaining worlds to FrontEnd world
            mWorldList.prepend(SWorldInfo());
            pInfo = &mWorldList.front();
            pInfo->WorldName = "FrontEnd";

            for (TResourceIterator<EResourceType::World> It; It; ++It)
            {
                if (UsedWorlds.find(It->ID()) == UsedWorlds.end())
                    pInfo->Areas.push_back(*It);
            }

            // Sort FrontEnd world
            std::sort( pInfo->Areas.begin(), pInfo->Areas.end(), [](const CResourceEntry *pA, const CResourceEntry *pB) {
                return pA->UppercaseName() < pB->UppercaseName();
            });
        }
    }

    endResetModel();
}

void CWorldTreeModel::OnMapChanged()
{
    // Flag all data as changed to ensure the font updates correctly based on which areas are loaded
    // note we don't know which areas used to be loaded, so flagging those specific indices isn't an option
    const int MaxRow = rowCount(QModelIndex()) - 1;
    const int MaxCol = columnCount(QModelIndex()) - 1;
    emit dataChanged(index(0, 0, QModelIndex()), index(MaxRow, MaxCol, QModelIndex()));
}

// ************ PROXY MODEL ************
bool CWorldTreeProxyModel::lessThan(const QModelIndex& rkSourceLeft, const QModelIndex& rkSourceRight) const
{
    const CWorldTreeModel *pModel = qobject_cast<CWorldTreeModel*>(sourceModel());
    ASSERT(pModel != nullptr);

    if (pModel->IndexIsWorld(rkSourceLeft))
    {
        ASSERT(pModel->IndexIsWorld(rkSourceRight));
        const bool IsLessThan = (rkSourceLeft.row() < rkSourceRight.row());
        return (sortOrder() == Qt::AscendingOrder ? IsLessThan : !IsLessThan);
    }

    return pModel->data(rkSourceLeft, Qt::DisplayRole).toString().toUpper() < pModel->data(rkSourceRight, Qt::DisplayRole).toString().toUpper();
}

bool CWorldTreeProxyModel::filterAcceptsRow(int SourceRow, const QModelIndex& rkSourceParent) const
{
    // Always accept worlds
    if (!rkSourceParent.isValid() || mFilterString.isEmpty())
        return true;

    const CWorldTreeModel *pModel = qobject_cast<CWorldTreeModel*>(sourceModel());
    ASSERT(pModel != nullptr);

    for (int iCol = 0; iCol < pModel->columnCount(rkSourceParent); iCol++)
    {
        const QModelIndex Index = pModel->index(SourceRow, iCol, rkSourceParent);
        if (pModel->data(Index, Qt::DisplayRole).toString().contains(mFilterString, Qt::CaseInsensitive))
            return true;
    }

    return false;
}
