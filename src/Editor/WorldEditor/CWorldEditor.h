#ifndef CWORLDEDITOR_H
#define CWORLDEDITOR_H

#include "Editor/CGizmo.h"
#include "Editor/CSceneViewport.h"
#include "Editor/INodeEditor.h"
#include "Editor/NDolphinIntegration.h"

#include <Core/Resource/TResPtr.h>

#include <array>
#include <memory>

namespace Ui {
class CWorldEditor;
}

class CCollisionRenderSettingsDialog;
class CGameArea;
class CGeneratePropertyNamesDialog;
class CLinkDialog;
class CLogDialog;
class CPoiMapSidebar;
class CScriptEditSidebar;
class CScriptObject;
class CTweakEditor;
class CWorld;
class CWorldEditorSidebar;
class CWorldInfoSidebar;
struct SRayIntersection;

class QAction;
class QButtonGroup;
class QMenu;
class QVBoxLayout;

enum EWorldEditorMode
{
    eWEM_EditWorldInfo,
    eWEM_EditScript,
    eWEM_EditPOIMappings
};

class CWorldEditor : public INodeEditor
{
    Q_OBJECT
    static constexpr int mskMaxRecentProjects = 10;

    std::unique_ptr<Ui::CWorldEditor> ui;
    QMenu* mpOpenRecentMenu;
    std::array<QAction*, mskMaxRecentProjects> mRecentProjectsActions;

    TResPtr<CWorld> mpWorld;
    TResPtr<CGameArea> mpArea;

    CCollisionRenderSettingsDialog* mpCollisionDialog;
    CLinkDialog* mpLinkDialog;
    CGeneratePropertyNamesDialog* mpGeneratePropertyNamesDialog;
    CTweakEditor* mpTweakEditor;

    bool mIsMakingLink = false;
    CScriptObject* mpNewLinkSender = nullptr;
    CScriptObject* mpNewLinkReceiver = nullptr;

    // Quickplay
    QAction* mpQuickplayAction;
    SQuickplayParameters mQuickplayParms;

    // Sidebars
    QVBoxLayout* mpRightSidebarLayout;
    CWorldEditorSidebar* mpCurSidebar;

    QButtonGroup* mpEditModeButtonGroup;
    CWorldInfoSidebar* mpWorldInfoSidebar;
    CScriptEditSidebar* mpScriptSidebar;
    CPoiMapSidebar* mpPoiMapSidebar;

    QAction* mpPoiMapAction;

    CLogDialog* mpLogDialog = nullptr;

public:
    explicit CWorldEditor(QWidget *parent = nullptr);
    ~CWorldEditor() override;

    bool CloseWorld();
    bool SetArea(CWorld *pWorld, int AreaIndex);
    void ResetCamera();
    bool HasAnyScriptNodesSelected() const;
    bool IsQuickplayEnabled() const;

    CWorld* ActiveWorld() const      { return mpWorld; }
    CGameArea* ActiveArea() const    { return mpArea; }
    EGame CurrentGame() const;
    CLinkDialog* LinkDialog() const  { return mpLinkDialog; }
    CGeneratePropertyNamesDialog* NameGeneratorDialog() const    { return mpGeneratePropertyNamesDialog; }
    CTweakEditor* TweakEditor()      { return mpTweakEditor; }
    CSceneViewport* Viewport() const override;

public slots:
    void EditorTick(float) override;
    bool Save() override;

    void Cut();
    void Copy();
    void Paste();

    void OpenProject();
    void OpenRecentProject();
    void ExportGame();
    void CloseProject();

    void About();

    void ChangeEditMode(int Mode);
    void ChangeEditMode(EWorldEditorMode Mode);
    void SetRenderingMergedWorld(bool RenderMerged);
    void OpenProjectSettings();

    void OnActiveProjectChanged(CGameProject *pProj);
    void OnPropertyModified(IProperty *pProp);
    void SetSelectionActive(bool Active);
    void SetSelectionInstanceNames(const QString& rkNewName, bool IsDone);
    void SetSelectionLayer(CScriptLayer *pLayer);
    void DeleteSelection();

    void UpdateOpenRecentActions();
    void UpdateWindowTitle();
    void UpdateStatusBar();
    void UpdateGizmoUI() override;
    void UpdateSelectionUI() override;
    void UpdateCursor();
    void UpdateNewLinkLine();

    void LaunchQuickplay();
    void LaunchQuickplayFromLocation(const CVector3f& Location, bool ForceAsSpawnPosition);

protected:
    QAction* AddEditModeButton(const QIcon& Icon, const QString& ToolTip, EWorldEditorMode Mode);
    void SetSidebar(CWorldEditorSidebar *pSidebar);
    void GizmoModeChanged(CGizmo::EGizmoMode Mode) override;

private slots:
    void OnClipboardDataModified();
    void OnSelectionModified();

    void OnLinkButtonToggled(bool Enabled);
    void OnLinkClick(const SRayIntersection& rkIntersect);
    void OnLinkEnd();
    void OnUnlinkClicked();

    void OnPickModeEnter(const QCursor& Cursor);
    void OnPickModeExit();
    void UpdateCameraOrbit();
    void OnCameraSpeedChange(double Speed);
    void OnTransformSpinBoxModified(const CVector3f& Value);
    void OnTransformSpinBoxEdited(const CVector3f& Value);

    void OnNodeAboutToBeDeleted(const CSceneNode* node);

    void SelectAllTriggered();
    void InvertSelectionTriggered();
    void ToggleDrawWorld();
    void ToggleDrawObjects();
    void ToggleDrawCollision();
    void ToggleDrawObjectCollision();
    void ToggleDrawLights();
    void ToggleDrawSky();
    void ToggleGameMode();
    void ToggleDisableAlpha();
    void SetNoLighting();
    void SetBasicLighting();
    void SetWorldLighting();
    void SetNoBloom();
    void SetBloomMaps();
    void SetFakeBloom();
    void SetBloom();
    void IncrementGizmo();
    void DecrementGizmo();
    void EditLayers();

    void OnShowLogClicked(bool show);

    void OnPlayFromHere(const CSceneNode* node, const CVector3f& position);

signals:
    void MapChanged(CWorld *pNewWorld, CGameArea *pNewArea);
    void LayersModified();
    void PropertyModified(IProperty *pProp, CScriptObject* pObject);
};

#endif // CWORLDEDITOR_H
