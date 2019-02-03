#ifndef INODEEDITOR_H
#define INODEEDITOR_H

#include "IEditor.h"
#include "CGizmo.h"
#include "CNodeSelection.h"
#include <Common/Math/ETransformSpace.h>
#include <Core/Scene/CScene.h>

#include <QMainWindow>
#include <QAction>
#include <QActionGroup>
#include <QComboBox>
#include <QList>

class INodeEditor : public IEditor
{
    Q_OBJECT

protected:
    // Node management
    CScene mScene;
    CNodeSelection *mpSelection;
    bool mSelectionLocked;

    // Gizmo
    CGizmo mGizmo;
    bool mShowGizmo;
    bool mGizmoHovering;
    bool mGizmoTransforming;
    ETransformSpace mTranslateSpace;
    ETransformSpace mRotateSpace;
    enum { eNotCloning, eReadyToClone, eCloning } mCloneState;

    // Gizmo widgets
    QActionGroup *mpGizmoGroup;
    QList<QAction*> mGizmoActions;
    QComboBox *mpTransformCombo;

    // Pick mode
    bool mPickMode;
    bool mExitOnInvalidPick;
    bool mEmitOnInvalidPick;
    bool mEmitOnButtonPress;
    FNodeFlags mAllowedPickNodes;
    CSceneNode *mpPickHoverNode;
    Qt::MouseButtons mPickButtons;
    Qt::KeyboardModifiers mPickModifiers;

public:
    explicit INodeEditor(QWidget *pParent = 0);
    virtual ~INodeEditor();
    CScene* Scene();
    CGizmo* Gizmo();
    bool IsGizmoVisible();
    void BeginGizmoTransform();
    void EndGizmoTransform();
    ETransformSpace CurrentTransformSpace();

    void SelectNode(CSceneNode *pNode);
    void BatchSelectNodes(QList<CSceneNode*> Nodes, bool ClearExistingSelection, const QString& rkCommandName = "Select");
    void DeselectNode(CSceneNode *pNode);
    void BatchDeselectNodes(QList<CSceneNode*> Nodes, const QString& rkCommandName = "Deselect");
    void ClearSelection();
    void ClearAndSelectNode(CSceneNode *pNode);
    void SelectAll(FNodeFlags NodeFlags);
    void InvertSelection(FNodeFlags NodeFlags);
    void SetSelectionLocked(bool Locked);
    bool HasSelection() const;
    CNodeSelection* Selection() const;

    void EnterPickMode(FNodeFlags AllowedNodes, bool ExitOnInvalidPick, bool EmitOnInvalidPick, bool EmitHoverOnButtonPress, QCursor Cursor = Qt::CrossCursor);
    void ExitPickMode();

    void NotifySelectionTransformed();
    virtual void NotifyNodeAboutToBeSpawned();
    virtual void NotifyNodeSpawned(CSceneNode *pNode);
    virtual void NotifyNodeAboutToBeDeleted(CSceneNode *pNode);
    virtual void NotifyNodeDeleted();

signals:
    void NodeAboutToBeSpawned();
    void NodeSpawned(CSceneNode *pNode);
    void NodeAboutToBeDeleted(CSceneNode *pNode);
    void NodeDeleted();
    void SelectionModified();
    void SelectionTransformed();

    void PickModeEntered(QCursor Cursor);
    void PickModeExited();
    void PickModeClick(const SRayIntersection& rkRayIntersect, QMouseEvent *pEvent);
    void PickModeHoverChanged(const SRayIntersection& rkRayIntersect, QMouseEvent *pEvent);

public slots:
    void OnSelectionModified();
    void OnGizmoMoved();
    virtual void UpdateGizmoUI() = 0;
    virtual void UpdateSelectionUI() = 0;

protected:
    virtual void GizmoModeChanged(CGizmo::EGizmoMode /*mode*/) {}

protected slots:
    void OnViewportClick(const SRayIntersection& rkRayIntersect, QMouseEvent *pEvent);
    void OnViewportInputProcessed(const SRayIntersection& rkRayIntersect, QMouseEvent *pEvent);

private:
    void UpdateTransformActionsEnabled();

private slots:
    void OnSelectObjectsTriggered();
    void OnTranslateTriggered();
    void OnRotateTriggered();
    void OnScaleTriggered();
    void OnTransformSpaceChanged(int SpaceIndex);
};

#endif // INODEEDITOR_H
