#ifndef CCONNECTIONMODEL_H
#define CCONNECTIONMODEL_H

#include <Core/Resource/Script/CScriptObject.h>
#include <QAbstractTableModel>

class CLinkModel : public QAbstractTableModel
{
    Q_OBJECT

    CScriptObject *mpObject = nullptr;
    ELinkType mType{ELinkType::Outgoing};

public:
    explicit CLinkModel(QObject *pParent = nullptr);

    void SetObject(CScriptObject *pObj);
    void SetConnectionType(ELinkType Type);
    int rowCount(const QModelIndex& rkParent = QModelIndex()) const override;
    int columnCount(const QModelIndex& rkParent) const override;
    QVariant data(const QModelIndex& rkIndex, int Role) const override;
    QVariant headerData(int Section, Qt::Orientation Orientation, int Role) const override;
};

#endif // CCONNECTIONMODEL_H
