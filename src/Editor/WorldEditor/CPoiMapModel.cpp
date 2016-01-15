#include "CPoiMapModel.h"
#include "CWorldEditor.h"
#include "Editor/UICommon.h"
#include <Core/Scene/CSceneIterator.h>
#include <Core/ScriptExtra/CPointOfInterestExtra.h>

CPoiMapModel::CPoiMapModel(CWorldEditor *pEditor, QObject *pParent /*= 0*/)
    : QAbstractListModel(pParent)
    , mpEditor(pEditor)
{
    mpEditor = pEditor;
    mpPoiToWorld = mpEditor->ActiveArea()->GetPoiToWorldMap();

    if (mpPoiToWorld)
    {
        // Create map of model nodes
        QMap<u32,CModelNode*> NodeMap;

        for (CSceneIterator It(mpEditor->Scene(), eModelNode, true); !It.DoneIterating(); ++It)
        {
            CModelNode *pNode = static_cast<CModelNode*>(*It);
            NodeMap[pNode->FindMeshID()] = pNode;
        }

        // Create list of mappings
        for (u32 iMap = 0; iMap < mpPoiToWorld->NumMeshLinks(); iMap++)
        {
            const CPoiToWorld::SPoiMeshLink& rkLink = mpPoiToWorld->MeshLinkByIndex(iMap);
            CScriptNode *pPOI = mpEditor->Scene()->ScriptNodeByID(rkLink.PoiInstanceID);

            if (!mPoiLookupMap.contains(pPOI))
            {
                SEditorPoiMap Map;
                Map.pPOI = pPOI;
                mMaps << Map;
                mPoiLookupMap[pPOI] = &mMaps.last();
            }

            if (NodeMap.contains(rkLink.MeshID))
                mPoiLookupMap[pPOI]->Models << NodeMap[rkLink.MeshID];
        }
    }
}

QVariant CPoiMapModel::headerData(int Section, Qt::Orientation Orientation, int Role) const
{
    if ( (Section == 0) && (Orientation == Qt::Horizontal) && (Role == Qt::DisplayRole) )
        return "PointOfInterest";

    return QVariant::Invalid;
}

int CPoiMapModel::rowCount(const QModelIndex& /*rkParent*/) const
{
    return mMaps.size();
}

QVariant CPoiMapModel::data(const QModelIndex& rkIndex, int Role) const
{
    if (rkIndex.row() < mMaps.size())
    {
        const SEditorPoiMap& rkMap = mMaps[rkIndex.row()];

        if (Role == Qt::DisplayRole)
        {
            if (rkMap.pPOI)
                return TO_QSTRING(rkMap.pPOI->Object()->InstanceName());
            else
                return "[INVALID POI]";
        }

        else if (Role == Qt::DecorationRole)
        {
            bool IsImportant = false;

            if (rkMap.pPOI)
            {
                // Get scan
                CScan *pScan = static_cast<CPointOfInterestExtra*>(rkMap.pPOI->Extra())->GetScan();

                if (pScan)
                    IsImportant = pScan->IsImportant();
            }

            if (IsImportant)
                return QIcon(":/icons/POI Important.png");
            else
                return QIcon(":/icons/POI Normal.png");
        }
    }

    return QVariant::Invalid;
}

CScriptNode* CPoiMapModel::PoiNodePointer(const QModelIndex& rkIndex) const
{
    if (rkIndex.row() < mMaps.size())
        return mMaps[rkIndex.row()].pPOI;

    return nullptr;
}

const QList<CModelNode*>& CPoiMapModel::GetPoiMeshList(const QModelIndex& rkIndex) const
{
    CScriptNode *pPOI = PoiNodePointer(rkIndex);
    return GetPoiMeshList(pPOI);
}

const QList<CModelNode*>& CPoiMapModel::GetPoiMeshList(CScriptNode *pPOI) const
{
    return mPoiLookupMap[pPOI]->Models;
}
