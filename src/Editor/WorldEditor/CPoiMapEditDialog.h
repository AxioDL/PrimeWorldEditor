#ifndef CPOIMAPEDITDIALOG_H
#define CPOIMAPEDITDIALOG_H

#include "CPoiMapModel.h"
#include "CPoiListDialog.h"

#include <QItemSelection>
#include <QMainWindow>
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

    // Viewport Picking
    enum EPickType
    {
        eNotPicking,
        eAddMeshes,
        eRemoveMeshes,
        eAddPOIs
    } mPickType;

    CModelNode *mpHoverModel;

    static const CColor skNormalColor;
    static const CColor skImportantColor;
    static const CColor skHoverColor;

public:
    explicit CPoiMapEditDialog(CWorldEditor *pEditor, QWidget *pParent = 0);
    ~CPoiMapEditDialog();
    void closeEvent(QCloseEvent *pEvent);
    void HighlightPoiModels(const QModelIndex& rkIndex);
    void UnhighlightPoiModels(const QModelIndex& rkIndex);
    void HighlightModel(const QModelIndex& rkIndex, CModelNode *pNode);
    void UnhighlightModel(CModelNode *pNode);
    bool IsImportant(const QModelIndex& rkIndex);
    void RevertModelOverlay(CModelNode *pModel);
    EPickType GetRealPickType(bool AltPressed) const;
    QModelIndex GetSelectedRow() const;

public slots:
    void Save();
    void SetHighlightSelected();
    void SetHighlightAll();
    void SetHighlightNone();
    void OnSelectionChanged(const QItemSelection& rkSelected, const QItemSelection& rkDeselected);
    void OnItemDoubleClick(QModelIndex Index);
    void OnUnmapAllPressed();

    void OnPickButtonClicked();
    void StopPicking();
    void OnInstanceListButtonClicked();
    void OnRemovePoiButtonClicked();
    void OnPoiPicked(const SRayIntersection& rkIntersect, QMouseEvent *pEvent);
    void OnModelPicked(const SRayIntersection& rkIntersect, QMouseEvent *pEvent);
    void OnModelHover(const SRayIntersection& rkIntersect, QMouseEvent *pEvent);

signals:
    void Closed();
};

#endif // CPOIMAPEDITDIALOG_H
