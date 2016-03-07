#ifndef CCONNECTIONMODEL_H
#define CCONNECTIONMODEL_H

#include <Core/Resource/Script/CScriptObject.h>
#include <QAbstractTableModel>

class CLinkModel : public QAbstractTableModel
{
    Q_OBJECT

    CScriptObject *mpObject;
    ELinkType mType;

public:
    explicit CLinkModel(QObject *pParent = 0);
    void SetObject(CScriptObject *pObj);
    void SetConnectionType(ELinkType type);
    int rowCount(const QModelIndex& parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
};

#endif // CCONNECTIONMODEL_H
