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

    enum EPickType
    {
        eNotPicking,
        eAddMeshes,
        eRemoveMeshes,
        eAddPOIs
    } mPickType;

    enum EPickTool
    {
        eNormalTool,
        eSprayCanTool
    } mPickTool;

    bool mHoverModelIsMapped;
    CModelNode *mpHoverModel;

    static const CColor skNormalColor;
    static const CColor skImportantColor;
    static const CColor skHoverColor;

public:
    explicit CPoiMapEditDialog(CWorldEditor *pEditor, QWidget *parent = 0);
    ~CPoiMapEditDialog();
    void closeEvent(QCloseEvent *pEvent);
    void HighlightPoiModels(const QModelIndex& rkIndex);
    void UnhighlightPoiModels(const QModelIndex& rkIndex);
    void HighlightModel(const QModelIndex& rkIndex, CModelNode *pNode);
    void UnhighlightModel(CModelNode *pNode);
    bool IsImportant(const QModelIndex& rkIndex);
    void RevertHoverModelOverlay();
    EPickType GetRealPickType(bool AltPressed) const;
    QModelIndex GetSelectedRow() const;

public slots:
    void Save();
    void SetHighlightSelected();
    void SetHighlightAll();
    void SetHighlightNone();
    void OnSelectionChanged(const QItemSelection& rkSelected, const QItemSelection& rkDeselected);
    void OnItemDoubleClick(QModelIndex Index);
    void OnToolComboBoxChanged(int NewIndex);

    void PickButtonClicked();
    void StopPicking();
    void OnNodePicked(const SRayIntersection& rkIntersect, QMouseEvent *pEvent);
    void OnNodeHover(const SRayIntersection& rkIntersect, QMouseEvent *pEvent);

signals:
    void Closed();
};

#endif // CPOIMAPEDITDIALOG_H
