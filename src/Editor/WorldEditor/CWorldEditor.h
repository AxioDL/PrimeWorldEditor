#ifndef CWORLDEDITOR_H
#define CWORLDEDITOR_H

#include "CCollisionRenderSettingsDialog.h"
#include "CEditorApplication.h"
#include "CLinkDialog.h"
#include "CPoiMapSidebar.h"
#include "CScriptEditSidebar.h"
#include "CWorldInfoSidebar.h"
#include "Editor/INodeEditor.h"
#include "Editor/CGeneratePropertyNamesDialog.h"
#include "Editor/CGizmo.h"
#include "Editor/CSceneViewport.h"

#include <Common/CTimer.h>
#include <Common/EKeyInputs.h>
#include <Math/CRay.h>
#include <Math/ETransformSpace.h>
#include <Core/Render/CRenderer.h>
#include <Core/Resource/Area/CGameArea.h>
#include <Core/Resource/CWorld.h>
#include <Core/Resource/TResPtr.h>
#include <Core/Scene/CScene.h>
#include <Core/SRayIntersection.h>

#include <QComboBox>
#include <QDir>
#include <QFile>
#include <QList>
#include <QMainWindow>
#include <QTimer>
#include <QUndoStack>

namespace Ui {
class CWorldEditor;
}

enum EWorldEditorMode
{
    eWEM_EditWorldInfo,
    eWEM_EditScript,
    eWEM_EditPOIMappings
};

class CWorldEditor : public INodeEditor
{
    Q_OBJECT
    static const int mskMaxRecentProjects = 10;

    Ui::CWorldEditor *ui;
    QMenu *mpOpenRecentMenu;
    QAction *mRecentProjectsActions[ mskMaxRecentProjects ];

    TResPtr<CWorld> mpWorld;
    TResPtr<CGameArea> mpArea;

    CCollisionRenderSettingsDialog *mpCollisionDialog;
    CLinkDialog *mpLinkDialog;
    CGeneratePropertyNamesDialog* mpGeneratePropertyNamesDialog;

    bool mIsMakingLink;
    CScriptObject *mpNewLinkSender;
    CScriptObject *mpNewLinkReceiver;

    QString mWorldDir;
    QString mPakFileList;
    QString mPakTarget;

    // Sidebars
    QVBoxLayout *mpRightSidebarLayout;
    CWorldEditorSidebar *mpCurSidebar;

    QButtonGroup *mpEditModeButtonGroup;
    CWorldInfoSidebar *mpWorldInfoSidebar;
    CScriptEditSidebar *mpScriptSidebar;
    CPoiMapSidebar *mpPoiMapSidebar;

    QAction *mpPoiMapAction;

public:
    explicit CWorldEditor(QWidget *parent = 0);
    ~CWorldEditor();
    void closeEvent(QCloseEvent *pEvent);
    bool CloseWorld();
    bool SetArea(CWorld *pWorld, int AreaIndex);
    bool CheckUnsavedChanges();
    void ResetCamera();
    bool HasAnyScriptNodesSelected() const;

    inline CWorld* ActiveWorld() const      { return mpWorld; }
    inline CGameArea* ActiveArea() const    { return mpArea; }
    inline EGame CurrentGame() const        { return gpEdApp->CurrentGame(); }
    inline CLinkDialog* LinkDialog() const  { return mpLinkDialog; }
    inline CGeneratePropertyNamesDialog* NameGeneratorDialog() const    { return mpGeneratePropertyNamesDialog; }
    CResourceBrowser* ResourceBrowser() const;
    CSceneViewport* Viewport() const;

    inline void SetWorldDir(QString WorldDir)       { mWorldDir = (QDir(WorldDir).exists() ? WorldDir : ""); }
    inline void SetPakFileList(QString FileList)    { mPakFileList = (QFile::exists(FileList) ? FileList : ""); }
    inline void SetPakTarget(QString PakTarget)     { mPakTarget = (QFile::exists(PakTarget) ? PakTarget : ""); }

    inline bool CanRepack() const { return !mWorldDir.isEmpty() && !mPakFileList.isEmpty() && !mPakTarget.isEmpty(); }

public slots:
    virtual void EditorTick(float);
    virtual void NotifyNodeAboutToBeDeleted(CSceneNode *pNode);

    void Cut();
    void Copy();
    void Paste();

    void OpenProject();
    void OpenRecentProject();
    bool Save();
    bool SaveAndRepack();
    void ExportGame();
    void CloseProject();

    void ChangeEditMode(int Mode);
    void ChangeEditMode(EWorldEditorMode Mode);
    void SetRenderingMergedWorld(bool RenderMerged);
    void OpenProjectSettings();

    void OnActiveProjectChanged(CGameProject *pProj);
    void OnLinksModified(const QList<CScriptObject*>& rkInstances);
    void OnPropertyModified(CScriptObject* pObject, IProperty *pProp);
    void SetSelectionActive(bool Active);
    void SetSelectionInstanceNames(const QString& rkNewName, bool IsDone);
    void SetSelectionLayer(CScriptLayer *pLayer);
    void DeleteSelection();

    void UpdateOpenRecentActions();
    void UpdateWindowTitle();
    void UpdateStatusBar();
    void UpdateGizmoUI();
    void UpdateSelectionUI();
    void UpdateCursor();
    void UpdateNewLinkLine();

protected:
    QAction* AddEditModeButton(QIcon Icon, QString ToolTip, EWorldEditorMode Mode);
    void SetSidebar(CWorldEditorSidebar *pSidebar);
    void GizmoModeChanged(CGizmo::EGizmoMode Mode);

private slots:
    void OnClipboardDataModified();
    void OnSelectionModified();

    void OnLinkButtonToggled(bool Enabled);
    void OnLinkClick(const SRayIntersection& rkIntersect);
    void OnLinkEnd();
    void OnUnlinkClicked();

    void OnUndoStackIndexChanged();
    void OnPickModeEnter(QCursor Cursor);
    void OnPickModeExit();
    void UpdateCameraOrbit();
    void OnCameraSpeedChange(double Speed);
    void OnTransformSpinBoxModified(CVector3f Value);
    void OnTransformSpinBoxEdited(CVector3f Value);

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
    void EditCollisionRenderSettings();
    void EditLayers();
    void GeneratePropertyNames();

signals:
    void MapChanged(CWorld *pNewWorld, CGameArea *pNewArea);
    void LayersModified();
    void InstancesLayerAboutToChange();
    void InstancesLayerChanged(const QList<CScriptNode*>& rkInstanceList);
    void InstanceLinksModified(const QList<CScriptObject*>& rkInstances);
    void PropertyModified(CScriptObject *pInst, IProperty *pProp);
};

#endif // CWORLDEDITOR_H
