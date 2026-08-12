#include "Editor/WorldEditor/CPoiMapModel.h"

#include "Editor/UICommon.h"
#include "Editor/WorldEditor/CWorldEditor.h"
#include <Common/Macros.h>
#include <Core/Resource/CPoiToWorld.h>
#include <Core/Resource/CWorld.h>
#include <Core/Resource/Scan/CScan.h>
#include <Core/Resource/Script/CScriptObject.h>
#include <Core/Scene/CModelNode.h>
#include <Core/Scene/CScriptNode.h>
#include <Core/ScriptExtra/CPointOfInterestExtra.h>

#include <QAbstractListModel>

CPoiMapModel::CPoiMapModel(CWorldEditor *pEditor, QObject *pParent)
    : QAbstractListModel(pParent)
    , mpEditor(pEditor)
{
    connect(pEditor, &CWorldEditor::MapChanged, this, &CPoiMapModel::OnMapChange);
}

CPoiMapModel::~CPoiMapModel() = default;

QVariant CPoiMapModel::headerData(int Section, Qt::Orientation Orientation, int Role) const
{
    if (Section == 0 && Orientation == Qt::Horizontal)
    {
        if (Role == Qt::DisplayRole)
            return tr("PointOfInterest");
    }
    return {};
}

int CPoiMapModel::rowCount(const QModelIndex& /*rkParent*/) const
{
    return mpPoiToWorld ? static_cast<int>(mpPoiToWorld->NumMappedPOIs()) : 0;
}

QVariant CPoiMapModel::data(const QModelIndex& rkIndex, int Role) const
{
    if (rkIndex.row() < rowCount())
    {
        const CPoiToWorld::SPoiMap* pkMap = mpPoiToWorld->MapByIndex(static_cast<size_t>(rkIndex.row()));
        const CScriptObject* pPOI = mpArea->InstanceByID(pkMap->PoiID);

        if (Role == Qt::DisplayRole)
        {
            if (pPOI)
                return TO_QSTRING(pPOI->InstanceName());

            return tr("[INVALID POI]");
        }

        if (Role == Qt::DecorationRole)
        {
            bool IsImportant = false;

            if (const CScriptNode* pNode = mpEditor->Scene()->NodeForInstance(pPOI))
            {
                // Get scan
                if (CScan* pScan = static_cast<CPointOfInterestExtra*>(pNode->Extra())->GetScan())
                    IsImportant = pScan->IsCriticalPropertyRef();
            }

            if (IsImportant)
                return QIcon(QStringLiteral(":/icons/POI Important.svg"));
            else
                return QIcon(QStringLiteral(":/icons/POI Normal.svg"));
        }
    }

    return {};
}

void CPoiMapModel::AddPOI(const CScriptNode* pPOI)
{
    if (mModelMap.contains(pPOI))
        return;

    const int NewIndex = static_cast<int>(mpPoiToWorld->NumMappedPOIs());
    beginInsertRows(QModelIndex(), NewIndex, NewIndex);

    mModelMap.insert(pPOI, {});
    mpPoiToWorld->AddPoi(pPOI->Instance()->InstanceID());

    endInsertRows();
}

void CPoiMapModel::AddMapping(const QModelIndex& rkIndex, CModelNode *pNode)
{
    const CScriptNode* pPOI = PoiNodePointer(rkIndex);
    AddPOI(pPOI);

    auto& list = mModelMap[pPOI];
    if (!list.contains(pNode))
        list.append(pNode);

    mpPoiToWorld->AddPoiMeshMap(pPOI->Instance()->InstanceID(), pNode->FindMeshID());
}

void CPoiMapModel::RemovePOI(const QModelIndex& rkIndex)
{
    beginRemoveRows(QModelIndex(), rkIndex.row(), rkIndex.row());
    const CScriptNode* pPOI = PoiNodePointer(rkIndex);
    const auto iter = mModelMap.find(pPOI);

    if (iter != mModelMap.cend())
        mModelMap.erase(iter);

    mpPoiToWorld->RemovePoi(pPOI->Instance()->InstanceID());
    endRemoveRows();
}

void CPoiMapModel::RemoveMapping(const QModelIndex& rkIndex, const CModelNode *pNode)
{
    const CScriptNode* pPOI = PoiNodePointer(rkIndex);
    const auto iter = mModelMap.find(pPOI);

    if (iter != mModelMap.cend())
        iter->removeOne(pNode);

    mpPoiToWorld->RemovePoiMeshMap(pPOI->Instance()->InstanceID(), pNode->FindMeshID());
}

bool CPoiMapModel::IsPoiTracked(const CScriptNode* pPOI) const
{
    return mModelMap.contains(pPOI);
}

bool CPoiMapModel::IsModelMapped(const QModelIndex& rkIndex, const CModelNode *pNode) const
{
    if (!rkIndex.isValid())
        return false;
    if (!pNode)
        return false;

    const auto* pPOI = PoiNodePointer(rkIndex);
    const auto iter = mModelMap.constFind(pPOI);
    if (iter == mModelMap.cend())
        return false;

    return iter->contains(pNode);
}

CScriptNode* CPoiMapModel::PoiNodePointer(const QModelIndex& rkIndex) const
{
    const auto rowIndex = static_cast<size_t>(rkIndex.row());
    ASSERT(rowIndex < mpPoiToWorld->NumMappedPOIs());

    const auto* pkMap = mpPoiToWorld->MapByIndex(rowIndex);
    return mpEditor->Scene()->NodeForInstanceID(pkMap->PoiID);
}

const QList<CModelNode*>& CPoiMapModel::GetPoiMeshList(const QModelIndex& rkIndex)
{
    const CScriptNode* pPOI = PoiNodePointer(rkIndex);
    return GetPoiMeshList(pPOI);
}

const QList<CModelNode*>& CPoiMapModel::GetPoiMeshList(const CScriptNode *pPOI)
{
    return mModelMap[pPOI];
}

void CPoiMapModel::OnMapChange(CWorld*, CGameArea *pArea)
{
    beginResetModel();
    mpArea = pArea;
    mpPoiToWorld = (pArea ? pArea->PoiToWorldMap() : nullptr);

    if (mpPoiToWorld)
    {
        // Create an ID -> Model Node lookup map
        QMap<uint32_t, CModelNode*> NodeMap;

        for (auto* node : mpEditor->Scene()->MakeNodeView(ENodeType::Model, true))
        {
            auto* model = static_cast<CModelNode*>(node);
            NodeMap[model->FindMeshID()] = model;
        }

        // Create internal model map
        for (size_t iPoi = 0; iPoi < mpPoiToWorld->NumMappedPOIs(); iPoi++)
        {
            const CPoiToWorld::SPoiMap *pkMap = mpPoiToWorld->MapByIndex(iPoi);

            if (const auto* pPoiNode = mpEditor->Scene()->NodeForInstanceID(pkMap->PoiID))
            {
                QList<CModelNode*> modelList;

                for (const auto modelID : pkMap->ModelIDs)
                {
                    if (NodeMap.contains(modelID))
                        modelList.push_back(NodeMap[modelID]);
                }

                mModelMap[pPoiNode] = std::move(modelList);
            }
        }
    }
    else
    {
        mModelMap.clear();
    }

    endResetModel();
}
