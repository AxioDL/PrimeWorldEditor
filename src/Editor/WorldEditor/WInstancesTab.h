#ifndef WINSTANCESTAB_H
#define WINSTANCESTAB_H

#include "CTypesInstanceModel.h"

#include <QWidget>
#include <QAction>
#include <QMenu>

class CWorldEditor;
class CSceneManager;

namespace Ui {
class WInstancesTab;
}

class WInstancesTab : public QWidget
{
    Q_OBJECT

    CWorldEditor *mpEditor;
    CSceneManager *mpScene;
    CTypesInstanceModel *mpLayersModel;
    CTypesInstanceModel *mpTypesModel;

    // Tree right-click context menu
    QMenu *mpTreeContextMenu;
    QAction *mpHideInstance;
    QAction *mpHideType;
    QAction *mpHideAllExceptType;

public:
    explicit WInstancesTab(QWidget *parent = 0);
    ~WInstancesTab();
    void SetEditor(CWorldEditor *pEditor, CSceneManager *pScene);
    void SetMaster(CMasterTemplate *pMaster);
    void SetArea(CGameArea *pArea);

private slots:
    void OnTreeClick(QModelIndex Index);
    void OnTreeDoubleClick(QModelIndex Index);
    void OnHideInstanceAction();
    void OnHideTypeAction();
    void OnHideAllExceptTypeAction();

private:
    Ui::WInstancesTab *ui;

    void ExpandTopLevelItems();
};

#endif // WINSTANCESTAB_H
