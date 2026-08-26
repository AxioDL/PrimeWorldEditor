#ifndef CLAYERMODEL_H
#define CLAYERMODEL_H

#include <Core/Resource/TResPtr.h>
#include <QAbstractListModel>

class CGameArea;
class CScriptLayer;

class CLayerModel : public QAbstractListModel
{
    Q_OBJECT

    TResPtr<CGameArea> mpArea;

public:
    explicit CLayerModel(QObject* pParent = nullptr);
    ~CLayerModel() override;

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    void SetArea(CGameArea* pArea);
    CScriptLayer* Layer(const QModelIndex& index) const;
};

#endif // CLAYERMODEL_H
