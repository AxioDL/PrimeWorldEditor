#ifndef WMODIFYTAB_H
#define WMODIFYTAB_H

#include "CLinkDialog.h"
#include "CLinkModel.h"
#include <Core/Scene/CSceneNode.h>
#include <Core/Scene/CScriptNode.h>

#include <QWidget>
#include <QScrollArea>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QMap>

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

    CLinkDialog *mpLinkDialog;

public:
    explicit WModifyTab(QWidget *pParent = 0);
    ~WModifyTab();
    void SetEditor(CWorldEditor *pEditor);
    void GenerateUI(QList<CSceneNode*>& Selection);
    void ClearUI();
    void CreateLinkDialog();
    void DeleteLinkDialog();

public slots:
    void OnWorldEditorClosed();
    void OnWorldSelectionTransformed();

    void OnAddOutgoingLinkClicked();
    void OnAddIncomingLinkClicked();
    void OnDeleteOutgoingLinkClicked();
    void OnDeleteIncomingLinkClicked();
    void OnLinkDialogAccept();
    void OnLinkDialogReject();

private:
    Ui::WModifyTab *ui;

private slots:
    void OnLinkTableDoubleClick(QModelIndex Index);
};

#endif // WMODIFYTAB_H
