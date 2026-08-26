#ifndef CPOIMAPSIDEBAR_H
#define CPOIMAPSIDEBAR_H

#include "Editor/WorldEditor/CPoiMapModel.h"
#include "Editor/WorldEditor/CWorldEditorSidebar.h"

#include <QSortFilterProxyModel>

#include <memory>

namespace Ui {
class CPoiMapSidebar;
}

class QMouseEvent;
struct SRayIntersection;

class CPoiMapSidebar : public CWorldEditorSidebar
{
    Q_OBJECT

public:
    enum class EHighlightMode
    {
        HighlightAll,
        HighlightNone,
        HighlightSelected
    };

    // Viewport Picking
    enum class EPickType
    {
        NotPicking,
        AddMeshes,
        RemoveMeshes,
        AddPOIs
    };

    explicit CPoiMapSidebar(CWorldEditor *pEditor);
    ~CPoiMapSidebar() override;

    void SidebarOpen() override;
    void SidebarClose() override;
    void HighlightPoiModels(const QModelIndex& rkIndex);
    void UnhighlightPoiModels(const QModelIndex& rkIndex);
    void HighlightModel(const QModelIndex& rkIndex, CModelNode *pNode);
    void UnhighlightModel(CModelNode *pNode);
    bool IsImportant(const QModelIndex& rkIndex) const;
    void RevertModelOverlay(CModelNode *pModel);
    EPickType GetRealPickType(bool AltPressed) const;
    QModelIndex GetSelectedRow() const;

private slots:
    void UpdateModelHighlights();
    void SetHighlightSelected();
    void SetHighlightAll();
    void SetHighlightNone();
    void OnSelectionChanged(const QItemSelection& rkSelected, const QItemSelection& rkDeselected);
    void OnItemDoubleClick(const QModelIndex& Index);
    void OnUnmapAllPressed();

    void OnPickButtonClicked();
    void StopPicking();
    void OnInstanceListButtonClicked();
    void OnRemovePoiButtonClicked();
    void OnPoiPicked(const SRayIntersection& rkIntersect, const QMouseEvent* pEvent);
    void OnModelPicked(const SRayIntersection& rkIntersect, const QMouseEvent* pEvent);
    void OnModelHover(const SRayIntersection& rkIntersect, const QMouseEvent* pEvent);

private:
    std::unique_ptr<Ui::CPoiMapSidebar> ui;

    CPoiMapModel mSourceModel;
    QSortFilterProxyModel mModel;
    EHighlightMode mHighlightMode{EHighlightMode::HighlightSelected};

    EPickType mPickType{EPickType::NotPicking};
    CModelNode* mpHoverModel = nullptr;
};

#endif // CPOIMAPEDITDIALOG_H
