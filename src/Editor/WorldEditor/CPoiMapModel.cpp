#include "CPoiMapModel.h"
#include "CWorldEditor.h"
#include "Editor/UICommon.h"
#include <Core/Scene/CSceneIterator.h>
#include <Core/ScriptExtra/CPointOfInterestExtra.h>

CPoiMapModel::CPoiMapModel(CWorldEditor *pEditor, QObject *pParent)
    : QAbstractListModel(pParent)
    , mpEditor(pEditor)
{
    connect(pEditor, &CWorldEditor::MapChanged, this, &CPoiMapModel::OnMapChange);
}

QVariant CPoiMapModel::headerData(int Section, Qt::Orientation Orientation, int Role) const
{
    if (Section == 0 && Orientation == Qt::Horizontal && Role == Qt::DisplayRole)
        return tr("PointOfInterest");

    return QVariant::Invalid;
}

int CPoiMapModel::rowCount(const QModelIndex& /*rkParent*/) const
{
    return mpPoiToWorld ? static_cast<int>(mpPoiToWorld->NumMappedPOIs()) : 0;
}

QVariant CPoiMapModel::data(const QModelIndex& rkIndex, int Role) const
{
    if (rkIndex.row() < rowCount(QModelIndex()))
    {
        const CPoiToWorld::SPoiMap *pkMap = mpPoiToWorld->MapByIndex(static_cast<size_t>(rkIndex.row()));
        CScriptObject *pPOI = mpArea->InstanceByID(pkMap->PoiID);

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
                const CScan *pScan = static_cast<CPointOfInterestExtra*>(pNode->Extra())->GetScan();

                if (pScan)
                    IsImportant = pScan->IsCriticalPropertyRef();
            }

            if (IsImportant)
                return QIcon(QStringLiteral(":/icons/POI Important.svg"));
            else
                return QIcon(QStringLiteral(":/icons/POI Normal.svg"));
        }
    }

    return QVariant::Invalid;
}

void CPoiMapModel::AddPOI(CScriptNode *pPOI)
{
    if (mModelMap.contains(pPOI))
        return;

    const int NewIndex = static_cast<int>(mpPoiToWorld->NumMappedPOIs());
    beginInsertRows(QModelIndex(), NewIndex, NewIndex);

    auto* pList = new QList<CModelNode*>();
    mModelMap[pPOI] = pList;
    mpPoiToWorld->AddPoi(pPOI->Instance()->InstanceID());

    endInsertRows();
}

void CPoiMapModel::AddMapping(const QModelIndex& rkIndex, CModelNode *pNode)
{
    CScriptNode *pPOI = PoiNodePointer(rkIndex);
    AddPOI(pPOI);

    QList<CModelNode*> *pList = mModelMap[pPOI];
    if (!pList->contains(pNode))
        pList->append(pNode);

    mpPoiToWorld->AddPoiMeshMap(pPOI->Instance()->InstanceID(), pNode->FindMeshID());
}

void CPoiMapModel::RemovePOI(const QModelIndex& rkIndex)
{
    beginRemoveRows(QModelIndex(), rkIndex.row(), rkIndex.row());
    CScriptNode *pPOI = PoiNodePointer(rkIndex);

    if (mModelMap.contains(pPOI))
    {
        delete mModelMap[pPOI];
        mModelMap.remove(pPOI);
    }

    mpPoiToWorld->RemovePoi(pPOI->Instance()->InstanceID());
    endRemoveRows();
}

void CPoiMapModel::RemoveMapping(const QModelIndex& rkIndex, CModelNode *pNode)
{
    CScriptNode *pPOI = PoiNodePointer(rkIndex);

    if (mModelMap.contains(pPOI))
    {
        QList<CModelNode*> *pList = mModelMap[pPOI];
        pList->removeOne(pNode);
        mpPoiToWorld->RemovePoiMeshMap(pPOI->Instance()->InstanceID(), pNode->FindMeshID());
    }
    else
    {
        mpPoiToWorld->RemovePoiMeshMap(pPOI->Instance()->InstanceID(), pNode->FindMeshID());
    }
}

bool CPoiMapModel::IsPoiTracked(CScriptNode *pPOI) const
{
    return mModelMap.contains(pPOI);
}

bool CPoiMapModel::IsModelMapped(const QModelIndex& rkIndex, CModelNode *pNode) const
{
    if (!pNode)
        return false;

    CScriptNode *pPOI = PoiNodePointer(rkIndex);

    if (mModelMap.contains(pPOI))
    {
        QList<CModelNode*> *pList = mModelMap[pPOI];
        return (pList->contains(pNode));
    }

    return false;
}

CScriptNode* CPoiMapModel::PoiNodePointer(const QModelIndex& rkIndex) const
{
    const auto rowIndex = static_cast<size_t>(rkIndex.row());

    if (rowIndex < mpPoiToWorld->NumMappedPOIs())
    {
        const CPoiToWorld::SPoiMap *pkMap = mpPoiToWorld->MapByIndex(rowIndex);
        return mpEditor->Scene()->NodeForInstanceID(pkMap->PoiID);
    }

    return nullptr;
}

const QList<CModelNode*>& CPoiMapModel::GetPoiMeshList(const QModelIndex& rkIndex) const
{
    CScriptNode *pPOI = PoiNodePointer(rkIndex);
    return GetPoiMeshList(pPOI);
}

const QList<CModelNode*>& CPoiMapModel::GetPoiMeshList(CScriptNode *pPOI) const
{
    return *mModelMap[pPOI];
}

void CPoiMapModel::OnMapChange(CWorld*, CGameArea *pArea)
{
    beginResetModel();
    mpArea = pArea;
    mpPoiToWorld = (pArea ? pArea->PoiToWorldMap() : nullptr);

    if (mpPoiToWorld)
    {
        // Create an ID -> Model Node lookup map
        QMap<uint32,CModelNode*> NodeMap;

        for (CSceneIterator It(mpEditor->Scene(), ENodeType::Model, true); !It.DoneIterating(); ++It)
        {
            CModelNode *pNode = static_cast<CModelNode*>(*It);
            NodeMap[pNode->FindMeshID()] = pNode;
        }

        // Create internal model map
        for (size_t iPoi = 0; iPoi < mpPoiToWorld->NumMappedPOIs(); iPoi++)
        {
            const CPoiToWorld::SPoiMap *pkMap = mpPoiToWorld->MapByIndex(iPoi);
            CScriptNode *pPoiNode = mpEditor->Scene()->NodeForInstanceID(pkMap->PoiID);

            if (pPoiNode)
            {
                auto* pModelList = new QList<CModelNode*>();

                for (const auto modelID : pkMap->ModelIDs)
                {
                    if (NodeMap.contains(modelID))
                        pModelList->push_back(NodeMap[modelID]);
                }

                mModelMap[pPoiNode] = pModelList;
            }
        }
    }
    else
    {
        QList<QList<CModelNode*>*> Lists = mModelMap.values();

        for (auto* list : Lists)
            delete list;

        mModelMap.clear();
    }

    endResetModel();
}
