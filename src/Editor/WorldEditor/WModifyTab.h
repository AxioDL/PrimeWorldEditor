#ifndef WMODIFYTAB_H
#define WMODIFYTAB_H

#include <QWidget>
#include <QScrollArea>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QMap>

#include "CLinkModel.h"
#include "../WPropertyEditor.h"
#include <Scene/CSceneNode.h>

class CWorldEditor;

namespace Ui {
class WModifyTab;
}

class WModifyTab : public QWidget
{
    Q_OBJECT

    CWorldEditor *mpWorldEditor;
    CSceneNode *mpSelectedNode;

    QMap<CScriptTemplate*, WPropertyEditor*> mCachedPropEditors;
    WPropertyEditor *mpCurPropEditor;
    CLinkModel *mpInLinkModel;
    CLinkModel *mpOutLinkModel;

public:
    explicit WModifyTab(QWidget *pParent = 0);
    ~WModifyTab();
    void SetEditor(CWorldEditor *pEditor);
    void GenerateUI(QList<CSceneNode*>& Selection);
    void ClearUI();
    void ClearCachedEditors();

private:
    Ui::WModifyTab *ui;

private slots:
    void OnLinkTableDoubleClick(QModelIndex Index);
};

#endif // WMODIFYTAB_H
