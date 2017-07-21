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
#if WIN32
    QString OpenInExplorerString = "Show in Explorer";
#elif __APPLE__
    QString OpenInExplorerString = "Show in Finder";
#else
    QString OpenInExplorerString = "Show in file manager";
#endif

    mpOpenAction = addAction("Open", this, SLOT(Open()));
    mpOpenInExternalAppAction = addAction("Open in external application", this, SLOT(OpenInExternalApp()));
    mpOpenInExplorerAction = addAction(OpenInExplorerString, this, SLOT(OpenInExplorer()));
    mpSelectFolderAction = addAction("Select folder", this, SLOT(SelectFolder()));
    addSeparator();
    mpRenameAction = addAction("Rename", this, SLOT(Rename()));
    mpShowReferencersAction = addAction("Show referencers", this, SLOT(ShowReferencers()));
    mpShowDependenciesAction = addAction("Show dependencies", this, SLOT(ShowDependencies()));
    addSeparator();
    mpCopyNameAction = addAction("Copy name", this, SLOT(CopyName()));
    mpCopyPathAction = addAction("Copy path", this, SLOT(CopyPath()));
    mpCopyIDAction = addAction("Copy asset ID", this, SLOT(CopyID()));
}

void CResourceTableContextMenu::ShowMenu(const QPoint& rkPos)
{
    // Fetch the entry/directory
    mProxyIndex = mpTable->indexAt(rkPos);

    if (mProxyIndex.isValid())
    {
        mIndex = mpProxy->mapToSource(mProxyIndex);
        mpEntry = mpModel->IndexEntry(mIndex);
        mpDirectory = mpModel->IndexDirectory(mIndex);

        // Show/hide menu options
        bool IsRes = (mpEntry != nullptr);
        mpOpenInExternalAppAction->setVisible(IsRes);
        mpSelectFolderAction->setVisible(mpModel->IsDisplayingAssetList());
        mpShowDependenciesAction->setVisible(IsRes);
        mpShowReferencersAction->setVisible(IsRes);
        mpCopyIDAction->setVisible(IsRes);

        // Exec menu
        QPoint GlobalPos = mpTable->viewport()->mapToGlobal(rkPos);
        exec(GlobalPos);
    }
}

// Menu Options
void CResourceTableContextMenu::Open()
{
    if (mpEntry)
        gpEdApp->EditResource(mpEntry);
    else
        mpBrowser->SetActiveDirectory(mpDirectory);
}

void CResourceTableContextMenu::OpenInExternalApp()
{
    ASSERT(mpEntry);
    UICommon::OpenInExternalApplication( TO_QSTRING(mpEntry->CookedAssetPath()) );
}

void CResourceTableContextMenu::OpenInExplorer()
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

void CResourceTableContextMenu::SelectFolder()
{
    CVirtualDirectory *pDir = (mpEntry ? mpEntry->Directory() : mpDirectory->Parent());
    mpBrowser->SetActiveDirectory(pDir);

    if (mpEntry)
        mpBrowser->SelectResource(mpEntry);
    else
        mpBrowser->SelectDirectory(mpDirectory);
}

void CResourceTableContextMenu::Rename()
{
    mpTable->edit(mProxyIndex);
}

void CResourceTableContextMenu::ShowReferencers()
{
    ASSERT(mpEntry);

    QList<CResourceEntry*> EntryList;

    for (CResourceIterator Iter(mpEntry->ResourceStore()); Iter; ++Iter)
    {
        if (Iter->Dependencies()->HasDependency(mpEntry->ID()))
            EntryList << *Iter;
    }

    if (!mpModel->IsDisplayingUserEntryList())
        mpBrowser->SetInspectedEntry(mpEntry);

    QString ListDesc = QString("Referencers of \"%1\"").arg( TO_QSTRING(mpEntry->CookedAssetPath().GetFileName()) );
    mpModel->DisplayEntryList(EntryList, ListDesc);
    mpBrowser->ClearFilters();
}

void CResourceTableContextMenu::ShowDependencies()
{
    ASSERT(mpEntry);

    std::set<CAssetID> Dependencies;
    mpEntry->Dependencies()->GetAllResourceReferences(Dependencies);

    QList<CResourceEntry*> EntryList;

    for (auto Iter = Dependencies.begin(); Iter != Dependencies.end(); Iter++)
    {
        CResourceEntry *pEntry = mpEntry->ResourceStore()->FindEntry(*Iter);

        if (pEntry)
            EntryList << pEntry;
    }

    if (!mpModel->IsDisplayingUserEntryList())
        mpBrowser->SetInspectedEntry(mpEntry);

    QString ListDesc = QString("Dependencies of \"%1\"").arg( TO_QSTRING(mpEntry->CookedAssetPath().GetFileName()) );
    mpModel->DisplayEntryList(EntryList, ListDesc);
    mpBrowser->ClearFilters();
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
