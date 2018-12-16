#ifndef CPOIMAPSIDEBAR_H
#define CPOIMAPSIDEBAR_H

#include "CPoiMapModel.h"
#include "CPoiListDialog.h"
#include "CWorldEditorSidebar.h"

#include <QItemSelection>
#include <QMainWindow>
#include <QSortFilterProxyModel>

namespace Ui {
class CPoiMapSidebar;
}

class CPoiMapSidebar : public CWorldEditorSidebar
{
    Q_OBJECT
    Ui::CPoiMapSidebar *ui;

    enum class EHighlightMode
    {
        HighlightAll,
        HighlightNone,
        HighlightSelected
    };

    CPoiMapModel mSourceModel;
    QSortFilterProxyModel mModel;
    EHighlightMode mHighlightMode;

    // Viewport Picking
    enum class EPickType
    {
        NotPicking,
        AddMeshes,
        RemoveMeshes,
        AddPOIs
    } mPickType;

    CModelNode *mpHoverModel;

    static const CColor skNormalColor;
    static const CColor skImportantColor;
    static const CColor skHoverColor;

public:
    explicit CPoiMapSidebar(CWorldEditor *pEditor);
    ~CPoiMapSidebar();
    void SidebarOpen();
    void SidebarClose();
    void HighlightPoiModels(const QModelIndex& rkIndex);
    void UnhighlightPoiModels(const QModelIndex& rkIndex);
    void HighlightModel(const QModelIndex& rkIndex, CModelNode *pNode);
    void UnhighlightModel(CModelNode *pNode);
    bool IsImportant(const QModelIndex& rkIndex);
    void RevertModelOverlay(CModelNode *pModel);
    EPickType GetRealPickType(bool AltPressed) const;
    QModelIndex GetSelectedRow() const;

public slots:
    void UpdateModelHighlights();
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
};

#endif // CPOIMAPEDITDIALOG_H
