#ifndef WMODIFYTAB_H
#define WMODIFYTAB_H

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

public:
    explicit WModifyTab(QWidget *pParent = 0);
    ~WModifyTab();
    void SetEditor(CWorldEditor *pEditor);
    void GenerateUI(QList<CSceneNode*>& Selection);
    void ClearUI();

public slots:
    void OnWorldSelectionTransformed();
    void OnPropertyModified(const QModelIndex& rkIndex, bool IsDone);

private:
    Ui::WModifyTab *ui;

private slots:
    void OnLinkTableDoubleClick(QModelIndex Index);
};

#endif // WMODIFYTAB_H
