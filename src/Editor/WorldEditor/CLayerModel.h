#ifndef CLAYERMODEL_H
#define CLAYERMODEL_H

#include <Core/Resource/Area/CGameArea.h>
#include <QAbstractListModel>

class CLayerModel : public QAbstractListModel
{
    TResPtr<CGameArea> mpArea;

public:
    explicit CLayerModel(QObject *pParent = nullptr);
    ~CLayerModel() override;

    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    void SetArea(CGameArea *pArea);
    CScriptLayer* Layer(const QModelIndex& index) const;
};

#endif // CLAYERMODEL_H
