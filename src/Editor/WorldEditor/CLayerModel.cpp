#include "CLayerModel.h"
#include "Editor/UICommon.h"
#include <Core/Resource/Script/CScriptLayer.h>

CLayerModel::CLayerModel(QObject *pParent) : QAbstractListModel(pParent)
{
    mpArea = nullptr;
    mHasGenerateLayer = false;
}

CLayerModel::~CLayerModel()
{
}

int CLayerModel::rowCount(const QModelIndex& /*parent*/) const
{
    if (!mpArea) return 0;
    if (mHasGenerateLayer) return mpArea->GetScriptLayerCount() + 1;
    else return mpArea->GetScriptLayerCount();
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
    mHasGenerateLayer = (pArea->GetGeneratorLayer() != nullptr);
    emit layoutChanged();
}

CScriptLayer* CLayerModel::Layer(const QModelIndex& index) const
{
    if (!mpArea) return nullptr;
    u32 NumLayers = mpArea->GetScriptLayerCount();

    if (index.row() < (int) NumLayers)
        return mpArea->GetScriptLayer(index.row());
    if (mHasGenerateLayer && (index.row() == NumLayers))
        return mpArea->GetGeneratorLayer();

    return nullptr;
}
