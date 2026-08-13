#ifndef WMODIFYTAB_H
#define WMODIFYTAB_H

#include <Core/Resource/Script/CScriptObject.h>

#include <QList>
#include <QWidget>

#include <memory>

namespace Ui {
class WModifyTab;
}

class CLinkModel;
class CPropertyView;
class CSceneNode;
class CScriptObject;
class CWorldEditor;
class QAction;
class QMenu;
struct SRayIntersection;

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

    bool IsPicking() const { return mIsPicking; }
    CSceneNode* EditNode() const { return mpSelectedNode; }

private slots:
    void GenerateUI();
    void OnInstanceLinksModified(const QList<CScriptObject*>& rkInstances);
    void OnWorldSelectionTransformed();
    void OnMapChanged();

    void OnLinksSelectionModified();
    void OnAddLinkActionClicked(const QAction* pAction);
    void OnPickModeClick(const SRayIntersection& rkIntersect);
    void OnPickModeExit();
    void OnDeleteLinksClicked();
    void OnEditLinkClicked();

    void OnLinkTableDoubleClick(const QModelIndex& Index);

private:
    bool HasSelectedScriptNode() const;

    std::unique_ptr<Ui::WModifyTab> ui;
};

#endif // WMODIFYTAB_H
