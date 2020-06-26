#include "CScriptEditSidebar.h"
#include "WEditorProperties.h"
#include "WCreateTab.h"
#include "WModifyTab.h"
#include "WInstancesTab.h"
#include "CWorldEditor.h"

CScriptEditSidebar::CScriptEditSidebar(CWorldEditor *pEditor)
    : CWorldEditorSidebar(pEditor)
{
    QVBoxLayout *pLayout = new QVBoxLayout(this);
    pLayout->setContentsMargins(1, 1, 1, 1);
    pLayout->setSpacing(2);

    mpEditorProperties = new WEditorProperties(this);
    mpEditorProperties->SyncToEditor(pEditor);
    pLayout->addWidget(mpEditorProperties);

    mpTabWidget = new QTabWidget(this);
    mpCreateTab = new WCreateTab(pEditor, this);
    mpModifyTab = new WModifyTab(pEditor, this);
    mpInstancesTab = new WInstancesTab(pEditor, this);

    mpTabWidget->setIconSize(QSize(24, 24));
    mpTabWidget->addTab(mpCreateTab, QIcon(QStringLiteral(":/icons/Create.svg")), {});
    mpTabWidget->addTab(mpModifyTab, QIcon(QStringLiteral(":/icons/Modify.svg")), {});
    mpTabWidget->addTab(mpInstancesTab, QIcon(QStringLiteral(":/icons/Instances.svg")), {});
    pLayout->addWidget(mpTabWidget);
}
