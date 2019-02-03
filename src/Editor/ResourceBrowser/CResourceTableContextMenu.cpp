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
    , mpClickedEntry(nullptr)
    , mpClickedDirectory(nullptr)
{
    // Connect to the view
    connect(pView, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(ShowMenu(QPoint)));
}

void CResourceTableContextMenu::InitMenu()
{
    // Clear existing menu items
    clear();

    if (mpClickedEntry)
    {
    #if WIN32
        const QString kOpenInExplorerString = "Show in Explorer";
    #elif __APPLE__
        const QString kOpenInExplorerString = "Show in Finder";
    #else
        const QString kOpenInExplorerString = "Show in file manager";
    #endif

        addAction("Open", this, SLOT(Open()));
        addAction("Open in External Application", this, SLOT(OpenInExternalApp()));
        addAction(kOpenInExplorerString, this, SLOT(OpenInExplorer()));
        addSeparator();
    }

    if (mpClickedEntry || mpClickedDirectory)
    {
        addAction("Rename", this, SLOT(Rename()));

        if (mpModel->IsDisplayingAssetList())
        {
            addAction("Select Folder", this, SLOT(SelectFolder()));
        }

        if (mpClickedEntry)
        {
            addAction("Show Referencers", this, SLOT(ShowReferencers()));
            addAction("Show Dependencies", this, SLOT(ShowDependencies()));
        }
    }

    if (mpClickedEntry || mpClickedDirectory || !mSelectedIndexes.isEmpty())
    {
        addAction("Delete", this, SLOT(Delete()));
    }

    addSeparator();

    if (mpClickedEntry)
    {
        addAction("Copy Name", this, SLOT(CopyName()));
        addAction("Copy Path", this, SLOT(CopyPath()));
        addAction("Copy ID", this, SLOT(CopyID()));
        addSeparator();
    }

    QMenu* pCreate = addMenu("Create...");
    mpBrowser->AddCreateAssetMenuActions(pCreate);
}

void CResourceTableContextMenu::ShowMenu(const QPoint& rkPos)
{
    if (mpBrowser->CurrentStore() == nullptr)
        return;

    // Fetch the entry/directory
    mClickedProxyIndex = mpTable->indexAt(rkPos);

    if (mClickedProxyIndex.isValid())
    {
        mClickedIndex = mpProxy->mapToSource(mClickedProxyIndex);
        mpClickedEntry = mpModel->IndexEntry(mClickedIndex);
        mpClickedDirectory = mpModel->IndexDirectory(mClickedIndex);
    }
    else
    {
        mClickedIndex = QModelIndex();
        mpClickedEntry = nullptr;
        mpClickedDirectory = nullptr;
    }

    // Fetch the list of selected indexes
    QItemSelection Selection = mpProxy->mapSelectionToSource( mpTable->selectionModel()->selection() );
    mSelectedIndexes = Selection.indexes();

    InitMenu();

    // Exec menu
    QPoint GlobalPos = mpTable->viewport()->mapToGlobal(rkPos);
    exec(GlobalPos);
}

// Menu Options
void CResourceTableContextMenu::Open()
{
    if (mpClickedEntry)
        gpEdApp->EditResource(mpClickedEntry);
    else
        mpBrowser->SetActiveDirectory(mpClickedDirectory);
}

void CResourceTableContextMenu::OpenInExternalApp()
{
    ASSERT(mpClickedEntry);
    UICommon::OpenInExternalApplication( TO_QSTRING(mpClickedEntry->CookedAssetPath()) );
}

void CResourceTableContextMenu::OpenInExplorer()
{
    if (mpClickedEntry)
    {
        QString Path = TO_QSTRING( mpClickedEntry->CookedAssetPath() );
        UICommon::OpenContainingFolder(Path);
    }
    else
    {
        TString BasePath = mpBrowser->CurrentStore()->ResourcesDir();
        QString Path = TO_QSTRING( BasePath + mpClickedDirectory->FullPath() );
        UICommon::OpenContainingFolder(Path);
    }
}

void CResourceTableContextMenu::Rename()
{
    mpTable->edit(mClickedProxyIndex);
}

void CResourceTableContextMenu::SelectFolder()
{
    CVirtualDirectory *pDir = (mpClickedEntry ? mpClickedEntry->Directory() : mpClickedDirectory->Parent());
    mpBrowser->SetActiveDirectory(pDir);

    if (mpClickedEntry)
        mpBrowser->SelectResource(mpClickedEntry);
    else
        mpBrowser->SelectDirectory(mpClickedDirectory);
}

void CResourceTableContextMenu::ShowReferencers()
{
    ASSERT(mpClickedEntry);

    QList<CResourceEntry*> EntryList;

    for (CResourceIterator Iter(mpClickedEntry->ResourceStore()); Iter; ++Iter)
    {
        if (Iter->Dependencies()->HasDependency(mpClickedEntry->ID()))
            EntryList << *Iter;
    }

    if (!mpModel->IsDisplayingUserEntryList())
        mpBrowser->SetInspectedEntry(mpClickedEntry);

    QString ListDesc = QString("Referencers of \"%1\"").arg( TO_QSTRING(mpClickedEntry->CookedAssetPath().GetFileName()) );
    mpModel->DisplayEntryList(EntryList, ListDesc);
    mpBrowser->ClearFilters();
}

void CResourceTableContextMenu::ShowDependencies()
{
    ASSERT(mpClickedEntry);

    std::set<CAssetID> Dependencies;
    mpClickedEntry->Dependencies()->GetAllResourceReferences(Dependencies);

    QList<CResourceEntry*> EntryList;

    for (auto Iter = Dependencies.begin(); Iter != Dependencies.end(); Iter++)
    {
        CResourceEntry *pEntry = mpClickedEntry->ResourceStore()->FindEntry(*Iter);

        if (pEntry)
            EntryList << pEntry;
    }

    if (!mpModel->IsDisplayingUserEntryList())
        mpBrowser->SetInspectedEntry(mpClickedEntry);

    QString ListDesc = QString("Dependencies of \"%1\"").arg( TO_QSTRING(mpClickedEntry->CookedAssetPath().GetFileName()) );
    mpModel->DisplayEntryList(EntryList, ListDesc);
    mpBrowser->ClearFilters();
}

void CResourceTableContextMenu::Delete()
{
    // Create confirmation message
    uint NumResources = 0, NumDirectories = 0;

    foreach (const QModelIndex& kIndex, mSelectedIndexes)
    {
        if (mpModel->IsIndexDirectory(kIndex))
            NumDirectories++;
        else
            NumResources++;
    }

    if (NumResources == 0 && NumDirectories == 0)
        return;

    QString ConfirmMsg = QString("Are you sure you want to permanently delete ");

    if (NumResources > 0)
    {
        ConfirmMsg += QString("%d resource%s").arg(NumResources).arg(NumResources == 1 ? "" : "s");

        if (NumDirectories > 0)
        {
            ConfirmMsg += " and ";
        }
    }
    if (NumDirectories > 0)
    {
        ConfirmMsg += QString("%d %s").arg(NumDirectories).arg(NumDirectories == 1 ? "directory" : "directories");
    }

    // Allow the user to confirm the action before performing it
    if (UICommon::YesNoQuestion(mpBrowser, "Warning", ConfirmMsg))
    {
        //@todo this is wrong lol
        QList<CVirtualDirectory*> List;
        List << mpClickedDirectory;
        mpBrowser->DeleteDirectories(List);
    }
}

void CResourceTableContextMenu::CopyName()
{
    if (mpClickedEntry)
        gpEdApp->clipboard()->setText( TO_QSTRING(mpClickedEntry->Name()) );
    else
        gpEdApp->clipboard()->setText( TO_QSTRING(mpClickedDirectory->Name()) );
}

void CResourceTableContextMenu::CopyPath()
{
    if (mpClickedEntry)
        gpEdApp->clipboard()->setText( TO_QSTRING(mpClickedEntry->CookedAssetPath(true)) );
    else
        gpEdApp->clipboard()->setText( TO_QSTRING(mpClickedDirectory->FullPath()) );
}

void CResourceTableContextMenu::CopyID()
{
    ASSERT(mpClickedEntry);
    gpEdApp->clipboard()->setText( TO_QSTRING(mpClickedEntry->ID().ToString()) );
}
