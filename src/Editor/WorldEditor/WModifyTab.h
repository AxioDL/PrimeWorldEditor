#ifndef WMODIFYTAB_H
#define WMODIFYTAB_H

#include "CLinkDialog.h"
#include "CLinkModel.h"
#include "Editor/CNodeSelection.h"
#include "Editor/PropertyEdit/CPropertyView.h"
#include <Core/Scene/CSceneNode.h>
#include <Core/Scene/CScriptNode.h>

#include <QList>
#include <QMenu>
#include <QWidget>

#include <memory>

class CWorldEditor;

namespace Ui {
class WModifyTab;
}

class WModifyTab : public QWidget
{
    Q_OBJECT

    CWorldEditor *mpWorldEditor;
    CSceneNode *mpSelectedNode;

    CLinkModel *mpInLinkModel;
    CLinkModel *mpOutLinkModel;

    QMenu *mpAddLinkMenu;
    QAction *mpAddFromViewportAction;
    QAction *mpAddFromListAction;
    ELinkType mAddLinkType;
    bool mIsPicking = false;

public:
    explicit WModifyTab(CWorldEditor *pEditor, QWidget *pParent = nullptr);
    ~WModifyTab() override;

    void ClearUI();
    CPropertyView* PropertyView() const;

public slots:
    void GenerateUI();
    void OnInstanceLinksModified(const QList<CScriptObject*>& rkInstances);
    void OnWorldSelectionTransformed();
    void OnMapChanged();

    void OnLinksSelectionModified();
    void OnAddLinkActionClicked(QAction *pAction);
    void OnPickModeClick(const SRayIntersection& rkIntersect);
    void OnPickModeExit();
    void OnDeleteLinksClicked();
    void OnEditLinkClicked();

    bool IsPicking() const       { return mIsPicking; }
    CSceneNode* EditNode() const { return mpSelectedNode; }

private:
    std::unique_ptr<Ui::WModifyTab> ui;

private slots:
    void OnLinkTableDoubleClick(QModelIndex Index);
};

#endif // WMODIFYTAB_H
