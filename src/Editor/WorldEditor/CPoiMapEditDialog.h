#ifndef CPOIMAPEDITDIALOG_H
#define CPOIMAPEDITDIALOG_H

#include <QMainWindow>
#include "CPoiMapModel.h"
#include <QItemSelection>
#include <QSortFilterProxyModel>

namespace Ui {
class CPoiMapEditDialog;
}

class CPoiMapEditDialog : public QMainWindow
{
    Q_OBJECT
    Ui::CPoiMapEditDialog *ui;

    enum EHighlightMode
    {
        eHighlightAll,
        eHighlightNone,
        eHighlightSelected
    };

    CWorldEditor *mpEditor;
    CPoiMapModel mSourceModel;
    QSortFilterProxyModel mModel;
    EHighlightMode mHighlightMode;

public:
    explicit CPoiMapEditDialog(CWorldEditor *pEditor, QWidget *parent = 0);
    ~CPoiMapEditDialog();
    void closeEvent(QCloseEvent *) { emit Closed(); }
    void HighlightPoiModels(const QModelIndex& rkIndex);
    void UnhighlightPoiModels(const QModelIndex& rkIndex);

public slots:
    void SetHighlightSelected();
    void SetHighlightAll();
    void SetHighlightNone();
    void OnSelectionChanged(const QItemSelection& rkSelected, const QItemSelection& rkDeselected);
    void OnItemDoubleClick(QModelIndex Index);

signals:
    void Closed();
};

#endif // CPOIMAPEDITDIALOG_H
