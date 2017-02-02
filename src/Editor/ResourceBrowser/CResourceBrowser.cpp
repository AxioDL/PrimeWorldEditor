#include "CResourceBrowser.h"
#include "ui_CResourceBrowser.h"
#include "Editor/CEditorApplication.h"
#include <Core/GameProject/AssetNameGeneration.h>
#include <Core/GameProject/CAssetNameMap.h>
#include <QFileDialog>
#include <QMenu>
#include <QMessageBox>

CResourceBrowser::CResourceBrowser(QWidget *pParent)
    : QDialog(pParent)
    , mpUI(new Ui::CResourceBrowser)
    , mpSelectedEntry(nullptr)
    , mpStore(gpResourceStore)
    , mpSelectedDir(nullptr)
    , mSearching(false)
{
    mpUI->setupUi(this);
    setWindowFlags(windowFlags() | Qt::WindowMinimizeButtonHint);

    // Set up table models
    mpModel = new CResourceTableModel(this);
    mpProxyModel = new CResourceProxyModel(this);
    mpProxyModel->setSourceModel(mpModel);
    mpUI->ResourceTableView->setModel(mpProxyModel);

    QHeaderView *pHeader = mpUI->ResourceTableView->horizontalHeader();
    pHeader->setSectionResizeMode(0, QHeaderView::Stretch);
    pHeader->resizeSection(1, 215);
    pHeader->resizeSection(2, 75);

    // Set up directory tree model
    mpDirectoryModel = new CVirtualDirectoryModel(this);
    mpUI->DirectoryTreeView->setModel(mpDirectoryModel);

    RefreshResources();

    // Set up Import Names menu
    QMenu *pImportNamesMenu = new QMenu(this);
    mpUI->ImportNamesButton->setMenu(pImportNamesMenu);

    QAction *pImportFromContentsTxtAction = new QAction("Import from Pak Contents List", this);
    pImportNamesMenu->addAction(pImportFromContentsTxtAction);

#if !PUBLIC_RELEASE
    QAction *pGenerateAssetNamesAction = new QAction("Generate Asset Names", this);
    pImportNamesMenu->addAction(pGenerateAssetNamesAction);
#endif

    QAction *pImportFromAssetNameMapAction = new QAction("Import from Asset Name Map", this);
    pImportNamesMenu->addAction(pImportFromAssetNameMapAction);

    // Set up connections
    connect(mpUI->StoreComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(UpdateStore()));
    connect(mpUI->SearchBar, SIGNAL(textChanged(QString)), this, SLOT(OnSearchStringChanged()));
    connect(mpUI->SortComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(OnSortModeChanged(int)));
    connect(mpUI->DirectoryTreeView->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(OnDirectorySelectionChanged(QModelIndex,QModelIndex)));
    connect(mpUI->ResourceTableView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(OnDoubleClickTable(QModelIndex)));
    connect(mpUI->ResourceTableView->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(OnResourceSelectionChanged(QModelIndex, QModelIndex)));
    connect(pImportFromContentsTxtAction, SIGNAL(triggered()), this, SLOT(OnImportPakContentsTxt()));
    connect(pImportFromAssetNameMapAction, SIGNAL(triggered()), this, SLOT(OnImportNamesFromAssetNameMap()));
    connect(mpUI->ExportNamesButton, SIGNAL(clicked()), this, SLOT(ExportAssetNames()));
    connect(&mUpdateFilterTimer, SIGNAL(timeout()), this, SLOT(UpdateFilter()));

#if !PUBLIC_RELEASE
    connect(pGenerateAssetNamesAction, SIGNAL(triggered()), this, SLOT(OnGenerateAssetNames()));
#endif
}

CResourceBrowser::~CResourceBrowser()
{
    delete mpUI;
}

void CResourceBrowser::RefreshResources()
{
    // Fill resource table
    mpModel->FillEntryList(mpSelectedDir, mSearching);

    // Mark directories to span all three columns
    mpUI->ResourceTableView->clearSpans();

    for (u32 iDir = 0; iDir < mpModel->NumDirectories(); iDir++)
        mpUI->ResourceTableView->setSpan(iDir, 0, 1, 3);
}

void CResourceBrowser::UpdateDescriptionLabel()
{
    QString Desc;

    bool ValidDir = mpSelectedDir && !mpSelectedDir->IsRoot();
    QString Path = (ValidDir ? '\\' + TO_QSTRING(mpSelectedDir->FullPath()) : "");

    if (mSearching)
    {
        QString SearchText = mpUI->SearchBar->text();
        Desc = QString("Searching \"%1\"").arg(SearchText);

        if (ValidDir)
            Desc += QString(" in %1").arg(Path);
    }
    else
    {
        if (ValidDir)
            Desc = Path;
    }

    mpUI->TableDescriptionLabel->setText(Desc);
}

void CResourceBrowser::UpdateStore()
{
    int StoreIndex = mpUI->StoreComboBox->currentIndex();
    CResourceStore *pNewStore = (StoreIndex == 0 ? gpResourceStore : gpEditorStore);

    if (mpStore != pNewStore)
    {
        mpStore = pNewStore;

        // Refresh directory tree
        mpDirectoryModel->SetRoot(mpStore ? mpStore->RootDirectory() : nullptr);
        QModelIndex RootIndex = mpDirectoryModel->index(0, 0, QModelIndex());
        mpUI->DirectoryTreeView->expand(RootIndex);
        mpUI->DirectoryTreeView->clearSelection();
        OnDirectorySelectionChanged(QModelIndex(), QModelIndex());
    }
}

void CResourceBrowser::OnSortModeChanged(int Index)
{
    CResourceProxyModel::ESortMode Mode = (Index == 0 ? CResourceProxyModel::eSortByName : CResourceProxyModel::eSortBySize);
    mpProxyModel->SetSortMode(Mode);
}

void CResourceBrowser::OnSearchStringChanged()
{
    const int kUpdateWaitTime = 500;
    mUpdateFilterTimer.start(kUpdateWaitTime);
}

void CResourceBrowser::OnDirectorySelectionChanged(const QModelIndex& rkNewIndex, const QModelIndex& /*rkPrevIndex*/)
{
    if (rkNewIndex.isValid())
        mpSelectedDir = mpDirectoryModel->IndexDirectory(rkNewIndex);
    else
        mpSelectedDir = mpStore ? mpStore->RootDirectory() : nullptr;

    UpdateDescriptionLabel();
    RefreshResources();
}

void CResourceBrowser::OnDoubleClickTable(QModelIndex Index)
{
    QModelIndex SourceIndex = mpProxyModel->mapToSource(Index);

    // Directory - switch to the selected directory
    if (mpModel->IsIndexDirectory(SourceIndex))
    {
        CVirtualDirectory *pDir = mpModel->IndexDirectory(SourceIndex);
        QModelIndex Index = mpDirectoryModel->GetIndexForDirectory(pDir);
        mpUI->DirectoryTreeView->selectionModel()->setCurrentIndex(Index, QItemSelectionModel::ClearAndSelect);
    }

    // Resource - open resource for editing
    else
    {
        CResourceEntry *pEntry = mpModel->IndexEntry(SourceIndex);
        gpEdApp->EditResource(pEntry);
    }
}

void CResourceBrowser::OnResourceSelectionChanged(const QModelIndex& rkNewIndex, const QModelIndex& /*rkPrevIndex*/)
{
    QModelIndex SourceIndex = mpProxyModel->mapToSource(rkNewIndex);
    mpSelectedEntry = mpModel->IndexEntry(SourceIndex);
    emit SelectedResourceChanged(mpSelectedEntry);
}

void CResourceBrowser::OnImportPakContentsTxt()
{
    QStringList PathList = QFileDialog::getOpenFileNames(this, "Open pak contents list", "", "*.pak.contents.txt");
    if (PathList.isEmpty()) return;

    foreach(const QString& rkPath, PathList)
        mpStore->ImportNamesFromPakContentsTxt(TO_TSTRING(rkPath), false);

    RefreshResources();
}

void CResourceBrowser::OnGenerateAssetNames()
{
    GenerateAssetNames(mpStore->Project());
    RefreshResources();
}

void CResourceBrowser::OnImportNamesFromAssetNameMap()
{
    CAssetNameMap Map;
    Map.LoadAssetNames();

    for (CResourceIterator It(mpStore); It; ++It)
    {
        TString Dir, Name;

        if (Map.GetNameInfo(It->ID(), Dir, Name))
            It->Move(Dir.ToUTF16(), Name.ToUTF16());
    }

    mpStore->ConditionalSaveStore();
    RefreshResources();
}

void CResourceBrowser::ExportAssetNames()
{
    QString OutFile = QFileDialog::getSaveFileName(this, "Export asset name map", "../resources/gameinfo/", "*.xml");
    if (OutFile.isEmpty()) return;

    CAssetNameMap NameMap;
    NameMap.LoadAssetNames();
    NameMap.CopyFromStore(mpStore);
    NameMap.SaveAssetNames();
}

void CResourceBrowser::UpdateFilter()
{
    QString SearchText = mpUI->SearchBar->text();
    bool NewSearching = !SearchText.isEmpty();

    if (mSearching != NewSearching)
    {
        mSearching = NewSearching;
        RefreshResources();
    }

    UpdateDescriptionLabel();
    mpProxyModel->SetSearchString( TO_TWIDESTRING(mpUI->SearchBar->text()) );
}
