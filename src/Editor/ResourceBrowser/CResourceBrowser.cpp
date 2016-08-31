#include "CResourceBrowser.h"
#include "ui_CResourceBrowser.h"
#include "Editor/ModelEditor/CModelEditorWindow.h"
#include "Editor/CharacterEditor/CCharacterEditor.h"
#include <QMessageBox>

CResourceBrowser::CResourceBrowser(QWidget *pParent)
    : QDialog(pParent)
    , mpUI(new Ui::CResourceBrowser)
    , mpStore(gpResourceStore)
{
    mpUI->setupUi(this);
    setWindowFlags(windowFlags() | Qt::WindowMinimizeButtonHint);

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
    mpDirectoryModel->SetRoot(gpResourceStore ? gpResourceStore->RootDirectory() : nullptr);
    mpUI->DirectoryTreeView->setModel(mpDirectoryModel);
    mpUI->DirectoryTreeView->expand(mpDirectoryModel->index(0, 0, QModelIndex()));

    // Set up connections
    connect(mpUI->StoreComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(OnStoreChanged(int)));
    connect(mpUI->SearchBar, SIGNAL(textChanged(QString)), this, SLOT(OnSearchStringChanged()));
    connect(mpUI->SortComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(OnSortModeChanged(int)));
    connect(mpUI->DirectoryTreeView->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(OnDirectorySelectionChanged(QModelIndex,QModelIndex)));
    connect(mpUI->ResourceTableView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(OnDoubleClickResource(QModelIndex)));
}

CResourceBrowser::~CResourceBrowser()
{
    delete mpUI;
}

void CResourceBrowser::RefreshResources()
{
    mpModel->FillEntryList(mpStore);
}

void CResourceBrowser::OnStoreChanged(int Index)
{
    mpStore = (Index == 0 ? gpResourceStore : gpEditorStore);
    RefreshResources();

    mpDirectoryModel->SetRoot(mpStore ? mpStore->RootDirectory() : nullptr);
    QModelIndex RootIndex = mpDirectoryModel->index(0, 0, QModelIndex());
    mpUI->DirectoryTreeView->expand(RootIndex);
    mpUI->DirectoryTreeView->clearSelection();
    OnDirectorySelectionChanged(QModelIndex(), QModelIndex());
}

void CResourceBrowser::OnSortModeChanged(int Index)
{
    CResourceProxyModel::ESortMode Mode = (Index == 0 ? CResourceProxyModel::eSortByName : CResourceProxyModel::eSortBySize);
    mpProxyModel->SetSortMode(Mode);
}

void CResourceBrowser::OnSearchStringChanged()
{
    mpProxyModel->SetSearchString( TO_TWIDESTRING(mpUI->SearchBar->text()) );
}

void CResourceBrowser::OnDirectorySelectionChanged(const QModelIndex& rkNewIndex, const QModelIndex& /*rkPrevIndex*/)
{
    CVirtualDirectory *pDir = nullptr;

    if (rkNewIndex.isValid())
        pDir = mpDirectoryModel->IndexDirectory(rkNewIndex);

    mpProxyModel->SetDirectory(pDir);
}

void CResourceBrowser::OnDoubleClickResource(QModelIndex Index)
{
    QModelIndex SourceIndex = mpProxyModel->mapToSource(Index);
    CResourceEntry *pEntry = mpModel->IndexEntry(SourceIndex);

    if (pEntry->ResourceType() == eModel)
    {
        CModel *pModel = (CModel*) pEntry->Load();

        if (pModel)
        {
            CModelEditorWindow *pModelEd = new CModelEditorWindow(parentWidget());
            pModelEd->SetActiveModel(pModel);
            pModelEd->show();
        }
        else
            QMessageBox::warning(this, "Error", "Failed to load resource");
    }

    else if (pEntry->ResourceType() == eAnimSet)
    {
        CAnimSet *pSet = (CAnimSet*) pEntry->Load();

        if (pSet)
        {
            CCharacterEditor *pCharEd = new CCharacterEditor(parentWidget());
            pCharEd->SetActiveAnimSet(pSet);
            pCharEd->show();
        }
        else
            QMessageBox::warning(this, "Error", "Failed to load resource");
    }

    else
        QMessageBox::information(this, "Unsupported Resource", "The selected resource type is currently unsupported for editing.");
}
