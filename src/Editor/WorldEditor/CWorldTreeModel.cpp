#include "Editor/WorldEditor/CWorldTreeModel.h"

#include "Editor/CEditorApplication.h"
#include "Editor/UICommon.h"
#include "Editor/WorldEditor/CWorldEditor.h"
#include <Core/GameProject/CGameProject.h>
#include <Core/Resource/CWorld.h>

#include <QIcon>
#include <memory>

CWorldTreeModel::CWorldTreeModel(CWorldEditor *pEditor)
{
    connect(gpEdApp, &CEditorApplication::ActiveProjectChanged, this, &CWorldTreeModel::OnProjectChanged);
    connect(pEditor, &CWorldEditor::MapChanged, this, &CWorldTreeModel::OnMapChanged);
}

CWorldTreeModel::~CWorldTreeModel() = default;

int CWorldTreeModel::rowCount(const QModelIndex& rkParent) const
{
    if (!rkParent.isValid())
        return int(mWorldList.size());

    if (IndexIsWorld(rkParent))
        return int(mWorldList[rkParent.row()].Areas.size());

    return 0;
}

int CWorldTreeModel::columnCount(const QModelIndex&) const
{
    return 2;
}

QModelIndex CWorldTreeModel::index(int Row, int Column, const QModelIndex& rkParent) const
{
    if (!hasIndex(Row, Column, rkParent))
        return {};

    // World
    if (!rkParent.isValid())
        return createIndex(Row, Column, quint64((Row << 16) | 0xFFFF));

    // Area
    return createIndex(Row, Column, quint64((rkParent.row() << 16) | (Row & 0xFFFF)) );
}

QModelIndex CWorldTreeModel::parent(const QModelIndex& rkChild) const
{
    if (IndexIsWorld(rkChild))
        return {};

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

            const TString& AreaInternalName = pWorld->AreaInternalName(AreaIndex);
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
            return {};

        if (IndexIsWorld(rkIndex))
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

    return {};
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
    return {};
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

    return gpEdApp->ActiveProject()->ResourceStore()->FindEntry(AreaID);
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

            for (const CAssetID& rkID : WorldIDs)
            {
                CResourceEntry* pEntry = pProj->ResourceStore()->FindEntry(rkID);
                if (!pEntry)
                    continue;

                TResPtr<CWorld> pWorld = pEntry->Load();
                if (pWorld == nullptr)
                    continue;

                SWorldInfo Info;
                Info.WorldName = TO_QSTRING(pWorld->Name());
                Info.pWorld = pWorld;

                // Add areas
                for (size_t iArea = 0; iArea < pWorld->NumAreas(); iArea++)
                {
                    const CAssetID& AreaID = pWorld->AreaResourceID(iArea);
                    CResourceEntry *pAreaEntry = pWorld->Entry()->ResourceStore()->FindEntry(AreaID);
                    ASSERT(pAreaEntry);
                    Info.Areas.push_back(pAreaEntry);
                }

                mWorldList.push_back(std::move(Info));
            }

            // Sort in alphabetical order for MP3
            if (pProj->Game() >= EGame::Corruption)
            {
                std::ranges::sort(mWorldList, [](const SWorldInfo& rkA, const SWorldInfo& rkB) {
                    return rkA.WorldName.toUpper() < rkB.WorldName.toUpper();
                });
            }
        }
        else // DKCR - Get worlds from areas.lst
        {
            TString AreaListPath = pProj->DiscFilesystemRoot(false) + "areas.lst";

            // I really need a good text stream class at some point
            using FILEPtr = std::unique_ptr<FILE, decltype(&std::fclose)>;
            FILEPtr pAreaList{std::fopen(AreaListPath.CString(), "r"), std::fclose};
            SWorldInfo *pInfo = nullptr;
            std::set<CAssetID> UsedWorlds;

            while (!std::feof(pAreaList.get()))
            {
                char LineBuffer[256] = {};
                std::fgets(LineBuffer, sizeof(LineBuffer), pAreaList.get());
                const TString Line(LineBuffer);

                CAssetID WorldID;
                TString WorldName;
                const auto IDSplit = Line.IndexOf(' ');

                if (IDSplit != -1)
                {
                    // Get world ID
                    const TString IDString = (IDSplit == -1 ? "" : Line.SubString(2, IDSplit - 2));
                    WorldID = CAssetID::FromString(IDString);

                    // Get world name
                    const TString WorldPath = (IDSplit == -1 ? "" : Line.SubString(IDSplit + 1, Line.Size() - IDSplit - 1));
                    const auto UnderscoreIdx = WorldPath.IndexOf('_');
                    const auto WorldDirEnd = WorldPath.IndexOf("\\/", UnderscoreIdx);

                    if (UnderscoreIdx != -1 && WorldDirEnd != -1)
                        WorldName = WorldPath.SubString(UnderscoreIdx + 1, WorldDirEnd - UnderscoreIdx - 1);
                }

                if (WorldID.IsValid() && !WorldName.IsEmpty())
                {
                    if (CResourceEntry* pEntry = pProj->ResourceStore()->FindEntry(WorldID))
                    {
                        QString WorldNameQ = TO_QSTRING(WorldName);

                        if (!pInfo || pInfo->WorldName != WorldNameQ)
                        {
                            pInfo = &mWorldList.emplace_back();
                            pInfo->WorldName = std::move(WorldNameQ);
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
            pInfo->WorldName = QStringLiteral("FrontEnd");

            for (const auto& world : pProj->ResourceStore()->MakeTypedResourceView(EResourceType::World))
            {
                if (!UsedWorlds.contains(world->ID()))
                    pInfo->Areas.push_back(world.get());
            }

            // Sort FrontEnd world
            std::ranges::sort(pInfo->Areas, [](const CResourceEntry *pA, const CResourceEntry *pB) {
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
    const int MaxRow = rowCount() - 1;
    const int MaxCol = columnCount() - 1;
    emit dataChanged(index(0, 0), index(MaxRow, MaxCol));
}

// ************ PROXY MODEL ************
bool CWorldTreeProxyModel::lessThan(const QModelIndex& rkSourceLeft, const QModelIndex& rkSourceRight) const
{
    const auto* pModel = qobject_cast<CWorldTreeModel*>(sourceModel());
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
    if (!rkSourceParent.isValid())
        return true;

    const auto filterExpression = filterRegularExpression();
    if (filterExpression.pattern().isEmpty())
        return true;

    const auto* model = qobject_cast<CWorldTreeModel*>(sourceModel());
    ASSERT(model != nullptr);

    for (int column = 0; column < model->columnCount(rkSourceParent); column++)
    {
        const QModelIndex index = model->index(SourceRow, column, rkSourceParent);
        if (model->data(index, Qt::DisplayRole).toString().contains(filterExpression))
            return true;
    }

    return false;
}
