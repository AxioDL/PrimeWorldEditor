#ifndef INODEEDITOR_H
#define INODEEDITOR_H

#include "CGizmo.h"
#include <Math/ETransformSpace.h>
#include <Core/Scene/CScene.h>

#include <QMainWindow>
#include <QAction>
#include <QActionGroup>
#include <QComboBox>
#include <QList>
#include <QUndoStack>

class INodeEditor : public QMainWindow
{
    Q_OBJECT

protected:
    // Undo stack
    QUndoStack mUndoStack;
    QList<QAction*> mUndoActions;

    // Node management
    CScene mScene;
    QList<CSceneNode*> mSelection;
    FNodeFlags mSelectionNodeFlags;
    CAABox mSelectionBounds;
    bool mSelectionLocked;

    // Gizmo
    CGizmo mGizmo;
    bool mShowGizmo;
    bool mGizmoHovering;
    bool mGizmoTransforming;
    ETransformSpace mTranslateSpace;
    ETransformSpace mRotateSpace;

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
    QUndoStack* UndoStack();
    CScene* Scene();
    CGizmo* Gizmo();
    bool IsGizmoVisible();
    void BeginGizmoTransform();
    void EndGizmoTransform();
    ETransformSpace CurrentTransformSpace();

    void RecalculateSelectionBounds();
    void ExpandSelectionBounds(CSceneNode *pNode);
    void SelectNode(CSceneNode *pNode);
    void DeselectNode(CSceneNode *pNode);
    void ClearSelection();
    void ClearAndSelectNode(CSceneNode *pNode);
    void SelectAll(FNodeFlags NodeFlags);
    void InvertSelection(FNodeFlags NodeFlags);
    void SetSelectionLocked(bool Locked);
    bool HasSelection() const;
    const QList<CSceneNode*>& GetSelection() const;

    void EnterPickMode(FNodeFlags AllowedNodes, bool ExitOnInvalidPick, bool EmitOnInvalidPick, bool EmitHoverOnButtonPress, QCursor Cursor = Qt::CrossCursor);
    void ExitPickMode();

signals:
    void SelectionModified();
    void SelectionTransformed();

    void PickModeEntered(QCursor Cursor);
    void PickModeExited();
    void PickModeClick(const SRayIntersection& rkRayIntersect, QMouseEvent *pEvent);
    void PickModeHoverChanged(const SRayIntersection& rkRayIntersect, QMouseEvent *pEvent);

public slots:
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
    void OnTransformSpaceChanged(int spaceIndex);
    void OnSelectionModified();
};

#endif // INODEEDITOR_H
