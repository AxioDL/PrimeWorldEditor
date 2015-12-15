#ifndef CCONNECTIONMODEL_H
#define CCONNECTIONMODEL_H

#include <QAbstractTableModel>
#include <Resource/script/CScriptObject.h>

class CLinkModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum EConnectionType {
        eIncoming, eOutgoing
    };

private:
    CScriptObject *mpObject;
    EConnectionType mType;

public:
    explicit CLinkModel(QObject *pParent = 0);
    void SetObject(CScriptObject *pObj);
    void SetConnectionType(EConnectionType type);
    int rowCount(const QModelIndex& parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
};

#endif // CCONNECTIONMODEL_H
