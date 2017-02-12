#include "CScriptEditSidebar.h"
#include "WCreateTab.h"
#include "WModifyTab.h"
#include "WInstancesTab.h"
#include "CWorldEditor.h"

CScriptEditSidebar::CScriptEditSidebar(CWorldEditor *pEditor)
    : QTabWidget(pEditor)
{
    mpCreateTab = new WCreateTab(pEditor, this);
    mpModifyTab = new WModifyTab(pEditor, this);
    mpInstancesTab = new WInstancesTab(pEditor, this);

    addTab(mpCreateTab, QIcon(":/icons/Create.png"), "Create");
    addTab(mpModifyTab, QIcon(":/icons/Modify.png"), "Modify");
    addTab(mpInstancesTab, QIcon(":/icons/Instances.png"), "Instances");
}
