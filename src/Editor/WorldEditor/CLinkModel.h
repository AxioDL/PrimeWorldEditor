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
    void SetConnectionType(ELinkType Type);
    int rowCount(const QModelIndex& rkParent = QModelIndex()) const;
    int columnCount(const QModelIndex& rkParent) const;
    QVariant data(const QModelIndex& rkIndex, int Role) const;
    QVariant headerData(int Section, Qt::Orientation Orientation, int Role) const;
};

#endif // CCONNECTIONMODEL_H
