#include "CResourceTableContextMenu.h"
#include "CResourceBrowser.h"
#include "Editor/CEditorApplication.h"
#include <QClipboard>

CResourceTableContextMenu::CResourceTableContextMenu(CResourceBrowser *pBrowser, QTableView *pView, CResourceTableModel *pModel, CResourceProxyModel *pProxy)
    : QMenu(pView)
    , mpBrowser(pBrowser)
    , mpTable(pView)
    , mpModel(pModel)
    , mpProxy(pProxy)
    , mpEntry(nullptr)
    , mpDirectory(nullptr)
{
    // Connect to the view
    connect(pView, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(ShowMenu(QPoint)));

    // Create actions
    mpOpenAction = addAction("Open", this, SLOT(Open()));
    mpOpenInExternalAppAction = addAction("Open in external application", this, SLOT(OpenInExternalApp()));
    mpOpenContainingFolderAction = addAction("Open containing folder", this, SLOT(OpenContainingFolder()));
    addSeparator();
    mpCopyNameAction = addAction("Copy name", this, SLOT(CopyName()));
    mpCopyPathAction = addAction("Copy path", this, SLOT(CopyPath()));
    mpCopyIDAction = addAction("Copy asset ID", this, SLOT(CopyID()));
}

void CResourceTableContextMenu::ShowMenu(const QPoint& rkPos)
{
    // Fetch the entry/directory
    QModelIndex ProxyIndex = mpTable->indexAt(rkPos);
    mIndex = mpProxy->mapToSource(ProxyIndex);
    mpEntry = mpModel->IndexEntry(mIndex);
    mpDirectory = mpModel->IndexDirectory(mIndex);

    // Show/hide menu options
    bool IsRes = (mpEntry != nullptr);
    mpOpenInExternalAppAction->setVisible(IsRes);
    mpCopyIDAction->setVisible(IsRes);

    // Exec menu
    QPoint GlobalPos = mpTable->viewport()->mapToGlobal(rkPos);
    exec(GlobalPos);
}

// Menu Options
void CResourceTableContextMenu::Open()
{
    if (mpEntry)
        gpEdApp->EditResource(mpEntry);
    else
        mpBrowser->SelectDirectory(mpDirectory);
}

void CResourceTableContextMenu::OpenInExternalApp()
{
    ASSERT(mpEntry);
    UICommon::OpenInExternalApplication( TO_QSTRING(mpEntry->CookedAssetPath()) );
}

void CResourceTableContextMenu::OpenContainingFolder()
{
    if (mpEntry)
    {
        QString Path = TO_QSTRING( mpEntry->CookedAssetPath() );
        UICommon::OpenContainingFolder(Path);
    }
    else
    {
        TString BasePath = mpBrowser->CurrentStore()->ResourcesDir();
        QString Path = TO_QSTRING( BasePath + mpDirectory->FullPath() );
        UICommon::OpenContainingFolder(Path);
    }
}

void CResourceTableContextMenu::CopyName()
{
    if (mpEntry)
        gpEdApp->clipboard()->setText( TO_QSTRING(mpEntry->Name()) );
    else
        gpEdApp->clipboard()->setText( TO_QSTRING(mpDirectory->Name()) );
}

void CResourceTableContextMenu::CopyPath()
{
    if (mpEntry)
        gpEdApp->clipboard()->setText( TO_QSTRING(mpEntry->CookedAssetPath(true)) );
    else
        gpEdApp->clipboard()->setText( TO_QSTRING(mpDirectory->FullPath()) );
}

void CResourceTableContextMenu::CopyID()
{
    ASSERT(mpEntry);
    gpEdApp->clipboard()->setText( TO_QSTRING(mpEntry->ID().ToString()) );
}
