#ifndef CWORLDEDITOR_H
#define CWORLDEDITOR_H

#include "CLinkDialog.h"
#include "CPoiMapEditDialog.h"
#include "Editor/INodeEditor.h"
#include "Editor/CGizmo.h"
#include "Editor/CSceneViewport.h"

#include <Common/CTimer.h>
#include <Common/EKeyInputs.h>
#include <Math/CRay.h>
#include <Math/ETransformSpace.h>
#include <Core/Render/CRenderer.h>
#include <Core/Resource/CGameArea.h>
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

class CWorldEditor : public INodeEditor
{
    Q_OBJECT
    Ui::CWorldEditor *ui;

    TResPtr<CWorld> mpWorld;
    TResPtr<CGameArea> mpArea;
    QTimer mRefreshTimer;

    CLinkDialog *mpLinkDialog;
    CPoiMapEditDialog *mpPoiDialog;

    bool mIsMakingLink;
    CScriptObject *mpNewLinkSender;
    CScriptObject *mpNewLinkReceiver;

    QString mWorldDir;
    QString mPakFileList;
    QString mPakTarget;

public:
    explicit CWorldEditor(QWidget *parent = 0);
    ~CWorldEditor();
    void closeEvent(QCloseEvent *pEvent);
    void SetArea(CWorld *pWorld, CGameArea *pArea);
    bool CheckUnsavedChanges();
    bool HasAnyScriptNodesSelected() const;

    inline CGameArea* ActiveArea() const    { return mpArea; }
    inline EGame CurrentGame() const        { return mpArea ? mpArea->Version() : eUnknownVersion; }
    inline CLinkDialog* LinkDialog() const  { return mpLinkDialog; }
    CSceneViewport* Viewport() const;

    inline void SetWorldDir(QString WorldDir)       { mWorldDir = (QDir(WorldDir).exists() ? WorldDir : ""); }
    inline void SetPakFileList(QString FileList)    { mPakFileList = (QFile::exists(FileList) ? FileList : ""); }
    inline void SetPakTarget(QString PakTarget)     { mPakTarget = (QFile::exists(PakTarget) ? PakTarget : ""); }

    inline bool CanRepack() const { return !mWorldDir.isEmpty() && !mPakFileList.isEmpty() && !mPakTarget.isEmpty(); }

public slots:
    virtual void NotifyNodeAboutToBeDeleted(CSceneNode *pNode);

    void Cut();
    void Copy();
    void Paste();
    bool Save();
    bool SaveAndRepack();
    void OnLinksModified(const QList<CScriptObject*>& rkInstances);
    void OnPropertyModified(IProperty *pProp);
    void SetSelectionActive(bool Active);
    void SetSelectionInstanceNames(const QString& rkNewName, bool IsDone);
    void SetSelectionLayer(CScriptLayer *pLayer);
    void DeleteSelection();

    void UpdateStatusBar();
    void UpdateGizmoUI();
    void UpdateSelectionUI();
    void UpdateCursor();
    void UpdateNewLinkLine();

protected:
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
    void RefreshViewport();
    void UpdateCameraOrbit();
    void OnCameraSpeedChange(double Speed);
    void OnTransformSpinBoxModified(CVector3f Value);
    void OnTransformSpinBoxEdited(CVector3f Value);
    void OnClosePoiEditDialog();

    void SelectAllTriggered();
    void InvertSelectionTriggered();
    void ToggleDrawWorld();
    void ToggleDrawObjects();
    void ToggleDrawCollision();
    void ToggleDrawObjectCollision();
    void ToggleDrawLights();
    void ToggleDrawSky();
    void ToggleGameMode();
    void ToggleBackfaceCull();
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
    void EditPoiToWorldMap();

signals:
    void Closed();
    void LayersModified();
    void InstancesLayerAboutToChange();
    void InstancesLayerChanged(const QList<CScriptNode*>& rkInstanceList);
    void InstanceLinksModified(const QList<CScriptObject*>& rkInstances);
    void PropertyModified(CScriptObject *pInst, IProperty *pProp);
};

#endif // CWORLDEDITOR_H
