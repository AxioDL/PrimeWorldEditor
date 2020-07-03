#include "CLayerModel.h"
#include "Editor/UICommon.h"
#include <Core/Resource/Script/CScriptLayer.h>

CLayerModel::CLayerModel(QObject *pParent)
    : QAbstractListModel(pParent)
{
}

CLayerModel::~CLayerModel() = default;

int CLayerModel::rowCount(const QModelIndex& /*parent*/) const
{
    return mpArea ? static_cast<int>(mpArea->NumScriptLayers()) : 0;
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
    if (!mpArea)
        return nullptr;

    const size_t NumLayers = mpArea->NumScriptLayers();

    if (index.row() < static_cast<int>(NumLayers))
        return mpArea->ScriptLayer(static_cast<size_t>(index.row()));

    return nullptr;
}
