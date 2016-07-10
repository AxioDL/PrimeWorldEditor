#include "CResourceBrowser.h"
#include "ui_CResourceBrowser.h"

CResourceBrowser::CResourceBrowser(QWidget *pParent)
    : QDialog(pParent)
    , mpUI(new Ui::CResourceBrowser)
{
    mpUI->setupUi(this);

    // Set up table models
    mpModel = new CResourceTableModel(this);
    mpProxyModel = new CResourceProxyModel(this);
    mpProxyModel->setSourceModel(mpModel);
    mpUI->ResourceTableView->setModel(mpProxyModel);
    RefreshResources();

    QHeaderView *pHeader = mpUI->ResourceTableView->horizontalHeader();
    pHeader->setSectionResizeMode(0, QHeaderView::Stretch);
    pHeader->resizeSection(1, 215);
    pHeader->resizeSection(2, 75);

    // Set up directory tree model
    mpDirectoryModel = new CVirtualDirectoryModel(this);
    mpDirectoryModel->SetRoot(gpResourceStore->RootDirectory());
    mpUI->DirectoryTreeView->setModel(mpDirectoryModel);
    mpUI->DirectoryTreeView->expand(mpDirectoryModel->index(0, 0, QModelIndex()));

    // Set up connections
    connect(mpUI->SearchBar, SIGNAL(textChanged(QString)), this, SLOT(OnSearchStringChanged()));
    connect(mpUI->SortComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(OnSortModeChanged(int)));
    connect(mpUI->DirectoryTreeView->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(OnDirectorySelectionChanged(QModelIndex,QModelIndex)));
}

CResourceBrowser::~CResourceBrowser()
{
    delete mpUI;
}

void CResourceBrowser::RefreshResources()
{
    mpModel->FillEntryList(gpResourceStore);
}

void CResourceBrowser::OnSortModeChanged(int Index)
{
    CResourceProxyModel::ESortMode Mode = (Index == 0 ? CResourceProxyModel::eSortByName : CResourceProxyModel::eSortBySize);
    mpProxyModel->SetSortMode(Mode);
}

void CResourceBrowser::OnSearchStringChanged()
{
    mpProxyModel->SetSearchString( mpUI->SearchBar->text() );
}

void CResourceBrowser::OnDirectorySelectionChanged(const QModelIndex& rkNewIndex, const QModelIndex& /*rkPrevIndex*/)
{
    CVirtualDirectory *pDir = nullptr;

    if (rkNewIndex.isValid())
        pDir = mpDirectoryModel->IndexDirectory(rkNewIndex);

    mpProxyModel->SetDirectory(pDir);
}
