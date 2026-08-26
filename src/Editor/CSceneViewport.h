#ifndef CSCENEVIEWPORT_H
#define CSCENEVIEWPORT_H

#include "Editor/CBasicViewport.h"
#include "Editor/CGridRenderable.h"
#include "Editor/CLineRenderable.h"
#include "Editor/INodeEditor.h"

#include <Core/SRayIntersection.h>

class CInstanceID;
class CRenderer;

class CSceneViewport : public CBasicViewport
{
    Q_OBJECT

    INodeEditor *mpEditor = nullptr;
    CScene *mpScene = nullptr;
    std::unique_ptr<CRenderer> mpRenderer;
    bool mRenderingMergedWorld = true;

    // Scene interaction
    bool mGizmoHovering = false;
    bool mGizmoTransforming = false;
    SRayIntersection mRayIntersection;
    CSceneNode *mpHoverNode = nullptr;
    CVector3f mHoverPoint{CVector3f::Zero()};

    // Context Menu
    QMenu *mpContextMenu = nullptr;
    QAction *mpToggleSelectAction;
    QAction *mpHideSelectionSeparator;
    QAction *mpHideSelectionAction;
    QAction *mpHideUnselectedAction;
    QAction *mpHideHoverSeparator;
    QAction *mpHideHoverNodeAction;
    QAction *mpHideHoverTypeAction;
    QAction *mpHideHoverLayerAction;
    QAction *mpUnhideSeparator;
    QAction *mpUnhideAllAction;
    QAction *mpPlayFromHereSeparator;
    QAction *mpPlayFromHereAction;
    CSceneNode *mpMenuNode = nullptr;
    CVector3f mMenuPoint;

    QMenu *mpSelectConnectedMenu;
    QAction *mpSelectConnectedOutgoingAction;
    QAction *mpSelectConnectedIncomingAction;
    QAction *mpSelectConnectedAllAction;

    // Grid
    CGridRenderable mGrid;

    // Link Line
    bool mLinkLineEnabled;
    CLineRenderable mLinkLine;

public:
    explicit CSceneViewport(QWidget *pParent = nullptr);
    ~CSceneViewport() override;

    void SetScene(INodeEditor *pEditor, CScene *pScene);
    void SetShowWorld(bool Visible);
    void SetRenderMergedWorld(bool RenderMerged);
    FShowFlags ShowFlags() const;
    CRenderer* Renderer();
    CSceneNode* HoverNode();
    const CVector3f& HoverPoint() const;
    void CheckGizmoInput(const CRay& rkRay);
    SRayIntersection SceneRayCast(const CRay& rkRay);
    void ResetHover();
    bool IsHoveringGizmo() const;

    void SetLinkLineEnabled(bool Enable)                                     { mLinkLineEnabled = Enable; }
    void SetLinkLine(const CVector3f& rkPointA, const CVector3f& rkPointB)   { mLinkLine.SetPoints(rkPointA, rkPointB); }

signals:
    void InputProcessed(const SRayIntersection& rkIntersect, QMouseEvent *pEvent);
    void ViewportClick(const SRayIntersection& rkIntersect, QMouseEvent *pEvent);
    void GizmoMoved();
    void CameraOrbit();
    void PlayFromHere(const CSceneNode* node, const CVector3f& position);

protected:
    void keyPressEvent(QKeyEvent* pEvent) override;
    void keyReleaseEvent(QKeyEvent* pEvent) override;

    void CheckUserInput() override;
    void Paint() override;
    void ContextMenu(QContextMenuEvent* pEvent) override;
    void OnResize() override;
    void OnMouseClick(QMouseEvent* pEvent) override;
    void OnMouseRelease(QMouseEvent* pEvent) override;

private:
    void CreateContextMenu();
    QMouseEvent CreateMouseEvent() const;
    void FindConnectedObjects(CInstanceID InstanceID, bool SearchOutgoing, bool SearchIncoming, QList<CInstanceID>& rIDList);

private slots:
    // Menu Actions
    void OnToggleSelect();
    void OnSelectConnected();
    void OnHideSelection();
    void OnHideUnselected();
    void OnHideNode();
    void OnHideType();
    void OnHideLayer();
    void OnUnhideAll();
    void OnPlayFromHere();
    void OnContextMenuClose();
};

#endif // CSCENEVIEWPORT_H
