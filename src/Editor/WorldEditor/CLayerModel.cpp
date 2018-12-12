#include "CLayerModel.h"
#include "Editor/UICommon.h"
#include <Core/Resource/Script/CScriptLayer.h>

CLayerModel::CLayerModel(QObject *pParent)
    : QAbstractListModel(pParent)
    , mpArea(nullptr)
{
}

CLayerModel::~CLayerModel()
{
}

int CLayerModel::rowCount(const QModelIndex& /*parent*/) const
{
    return mpArea ? mpArea->NumScriptLayers() : 0;
}

QVariant CLayerModel::data(const QModelIndex &index, int role) const
{
    if (mpArea && (role == Qt::DisplayRole) && (index.row() < rowCount(QModelIndex())))
        return TO_QSTRING(Layer(index)->Name());

    return QVariant::Invalid;
}

void CLayerModel::SetArea(CGameArea *pArea)
{
    mpArea = pArea;
    emit layoutChanged();
}

CScriptLayer* CLayerModel::Layer(const QModelIndex& index) const
{
    if (!mpArea) return nullptr;
    uint32 NumLayers = mpArea->NumScriptLayers();

    if (index.row() < (int) NumLayers)
        return mpArea->ScriptLayer(index.row());

    return nullptr;
}
