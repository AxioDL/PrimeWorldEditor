#ifndef CPROPERTYVIEW_H
#define CPROPERTYVIEW_H

#include "CPropertyModel.h"
#include "CPropertyDelegate.h"
#include <Core/Resource/Script/CScriptObject.h>
#include <QTreeView>

class CPropertyView : public QTreeView
{
    Q_OBJECT

    CPropertyModel *mpModel;
    CPropertyDelegate *mpDelegate;
    CScriptObject *mpObject;

public:
    CPropertyView(QWidget *pParent = 0);
    void setModel(QAbstractItemModel *pModel);
    bool event(QEvent *pEvent);
    void SetObject(CScriptObject *pObj);
    void UpdateEditorProperties(const QModelIndex& rkParent);

    inline CPropertyModel* PropertyModel() const { return mpModel; }

public slots:
    void SetPersistentEditors(const QModelIndex& rkIndex);
    void ClosePersistentEditors(const QModelIndex& rkIndex);
    void OnPropertyModified(const QModelIndex& rkIndex, bool IsDone);

signals:
    void PropertyModified(const QModelIndex &rkIndex, bool IsDone);
};

#endif // CPROPERTYVIEW_H
