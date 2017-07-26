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
    mpOpenInExternalAppAction = addAction("Open in External Application", this, SLOT(OpenInExternalApp()));
    mpOpenInExplorerAction = addAction(OpenInExplorerString, this, SLOT(OpenInExplorer()));
    addSeparator();
    mpRenameAction = addAction("Rename", this, SLOT(Rename()));
    mpSelectFolderAction = addAction("Select Folder", this, SLOT(SelectFolder()));
    mpShowReferencersAction = addAction("Show Referencers", this, SLOT(ShowReferencers()));
    mpShowDependenciesAction = addAction("Show Dependencies", this, SLOT(ShowDependencies()));
    mpDeleteAction = addAction("Delete", this, SLOT(Delete()));
    addSeparator();
    mpCopyNameAction = addAction("Copy Name", this, SLOT(CopyName()));
    mpCopyPathAction = addAction("Copy Path", this, SLOT(CopyPath()));
    mpCopyIDAction = addAction("Copy Asset ID", this, SLOT(CopyID()));
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
        mpDeleteAction->setVisible(mpDirectory && mpDirectory->IsEmpty(true));
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

void CResourceTableContextMenu::Rename()
{
    mpTable->edit(mProxyIndex);
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

void CResourceTableContextMenu::Delete()
{
    ASSERT(mpDirectory && mpDirectory->IsEmpty(true));

    QList<CVirtualDirectory*> List;
    List << mpDirectory;
    mpBrowser->DeleteDirectories(List);
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
