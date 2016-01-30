#ifndef CPROPERTYVIEW_H
#define CPROPERTYVIEW_H

#include "CPropertyModel.h"
#include "CPropertyDelegate.h"
#include <QTreeView>

class CPropertyView : public QTreeView
{
    Q_OBJECT

    CPropertyModel *mpModel;
    CPropertyDelegate *mpDelegate;

public:
    CPropertyView(QWidget *pParent = 0);
    void setModel(QAbstractItemModel *pModel);
    bool event(QEvent *pEvent);
    void SetBaseStruct(CPropertyStruct *pStruct);

    inline CPropertyModel* PropertyModel() const { return mpModel; }

public slots:
    void SetPersistentEditors(const QModelIndex& rkIndex);

signals:
    void PropertyModified(const QModelIndex& rkIndex, bool IsDone);
};

#endif // CPROPERTYVIEW_H
