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
    bool mIsPicking;

public:
    explicit WModifyTab(CWorldEditor *pEditor, QWidget *pParent = 0);
    ~WModifyTab();
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

    inline bool IsPicking() const       { return mIsPicking; }
    inline CSceneNode* EditNode() const { return mpSelectedNode; }

private:
    Ui::WModifyTab *ui;

private slots:
    void OnLinkTableDoubleClick(QModelIndex Index);
};

#endif // WMODIFYTAB_H
