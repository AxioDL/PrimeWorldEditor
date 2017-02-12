#ifndef CSCRIPTEDITSIDEBAR_H
#define CSCRIPTEDITSIDEBAR_H

#include <QTabWidget>

class CWorldEditor;
class WEditorProperties;
class WCreateTab;
class WModifyTab;
class WInstancesTab;

class CScriptEditSidebar : public QWidget
{
    Q_OBJECT

    WEditorProperties *mpEditorProperties;
    QTabWidget *mpTabWidget;
    WCreateTab *mpCreateTab;
    WModifyTab *mpModifyTab;
    WInstancesTab *mpInstancesTab;

public:
    CScriptEditSidebar(CWorldEditor *pEditor);

    // Accessors
    inline WCreateTab* CreateTab() const        { return mpCreateTab; }
    inline WModifyTab* ModifyTab() const        { return mpModifyTab; }
    inline WInstancesTab* InstancesTab() const  { return mpInstancesTab; }
};

#endif // CSCRIPTEDITSIDEBAR_H
