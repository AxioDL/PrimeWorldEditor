#ifndef WINSTANCESTAB_H
#define WINSTANCESTAB_H

#include "CInstancesProxyModel.h"
#include "CInstancesModel.h"

#include <QWidget>
#include <QAction>
#include <QMenu>

#include <memory>

class CWorldEditor;
class CScene;

namespace Ui {
class WInstancesTab;
}

class WInstancesTab : public QWidget
{
    Q_OBJECT

    CWorldEditor *mpEditor;
    CScene *mpScene;
    CInstancesModel *mpLayersModel;
    CInstancesModel *mpTypesModel;
    CInstancesProxyModel mLayersProxyModel;
    CInstancesProxyModel mTypesProxyModel;

    // Tree right-click context menu
    QMenu *mpTreeContextMenu;
    QAction *mpHideInstance;
    QAction *mpHideType;
    QAction *mpHideAllTypes;
    QAction *mpHideAllExceptType;
    QAction *mpSeparator;
    QAction *mpUnhideAllTypes;
    QAction *mpUnhideAll;

    QModelIndex mMenuIndex;
    CScriptNode *mpMenuObject;
    CScriptLayer *mpMenuLayer;
    CScriptTemplate *mpMenuTemplate;
    CInstancesModel::EIndexType mMenuIndexType;

public:
    explicit WInstancesTab(CWorldEditor *pEditor, QWidget *parent = nullptr);
    ~WInstancesTab() override;

private slots:
    void OnTreeClick(QModelIndex Index);
    void OnTreeDoubleClick(QModelIndex Index);

    void OnTreeContextMenu(QPoint Pos);
    void OnHideInstanceAction();
    void OnHideTypeAction();
    void OnHideAllTypesAction();
    void OnHideAllExceptTypeAction();
    void OnUnhideAllTypes();
    void OnUnhideAll();

    void ExpandTopLevelItems();

private:
    std::unique_ptr<Ui::WInstancesTab> ui;
};

#endif // WINSTANCESTAB_H
