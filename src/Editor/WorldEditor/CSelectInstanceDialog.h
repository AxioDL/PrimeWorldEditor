#ifndef CSELECTINSTANCEDIALOG_H
#define CSELECTINSTANCEDIALOG_H

#include "CInstancesModel.h"
#include "CInstancesProxyModel.h"
#include <QDialog>
#include <QSortFilterProxyModel>

namespace Ui {
class CSelectInstanceDialog;
}

class CSelectInstanceDialog : public QDialog
{
    Q_OBJECT

    CWorldEditor *mpEditor;
    CInstancesModel mLayersModel;
    CInstancesModel mTypesModel;
    CInstancesProxyModel mLayersProxyModel;
    CInstancesProxyModel mTypesProxyModel;

    bool mValidSelection;
    CScriptObject *mpLayersInst;
    CScriptObject *mpTypesInst;

    Ui::CSelectInstanceDialog *ui;

public:
    explicit CSelectInstanceDialog(CWorldEditor *pEditor, QWidget *pParent = 0);
    ~CSelectInstanceDialog();

    CScriptObject* SelectedInstance() const;

public slots:
    void OnTabChanged(int NewTabIndex);
    void OnTreeClicked(QModelIndex Index);
    void OnTreeDoubleClicked(QModelIndex Index);
};

#endif // CSELECTINSTANCEDIALOG_H
