#include "CResourceTableContextMenu.h"
#include "CResourceBrowser.h"
#include "Editor/CEditorApplication.h"

#include <Core/Resource/Scan/CScan.h>

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

    // Asset-specific
    if (mpClickedEntry)
    {
        switch (mpClickedEntry->ResourceType())
        {
        case EResourceType::StringTable:
            addAction("Create Scan", this, SLOT(CreateSCAN()));
            break;
        }
    }
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
    QVector<CResourceEntry*> Resources;
    QVector<CVirtualDirectory*> Directories;

    foreach (const QModelIndex& kIndex, mSelectedIndexes)
    {
        if (mpModel->IsIndexDirectory(kIndex))
            Directories << mpModel->IndexDirectory(kIndex);
        else
            Resources << mpModel->IndexEntry(kIndex);
    }

    mpBrowser->Delete(Resources, Directories);
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


// Asset Specific
void CResourceTableContextMenu::CreateSCAN()
{
    // Create a SCAN asset to go along with a selected STRG asset
    ASSERT( mpClickedEntry && mpClickedEntry->ResourceType() == EResourceType::StringTable );

    CResourceEntry* pNewEntry = mpBrowser->CreateNewResource(EResourceType::Scan,
                                                             mpClickedEntry->Name(),
                                                             mpClickedEntry->Directory());

    if (pNewEntry)
    {
        CScan* pScan = (CScan*) pNewEntry->Load();
        pScan->ScanStringPropertyRef().Set( mpClickedEntry->ID() );
        pNewEntry->Save();
    }
}
