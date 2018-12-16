#include "CSelectInstanceDialog.h"
#include "ui_CSelectInstanceDialog.h"

CSelectInstanceDialog::CSelectInstanceDialog(CWorldEditor *pEditor, QWidget *pParent)
    : QDialog(pParent)
    , ui(new Ui::CSelectInstanceDialog)
    , mpEditor(pEditor)
    , mLayersModel(pEditor, this)
    , mTypesModel(pEditor, this)
    , mValidSelection(false)
    , mpLayersInst(nullptr)
    , mpTypesInst(nullptr)
{
    ui->setupUi(this);

    mLayersModel.SetModelType(CInstancesModel::EInstanceModelType::Layers);
    mLayersModel.SetShowColumnEnabled(false);

    mTypesModel.SetModelType(CInstancesModel::EInstanceModelType::Types);
    mTypesModel.SetShowColumnEnabled(false);

    int Col0Width = ui->LayersTreeView->width() * 0.9;
    int Col1Width = ui->LayersTreeView->width() * 0.1;
    mLayersProxyModel.setSourceModel(&mLayersModel);
    ui->LayersTreeView->setModel(&mLayersProxyModel);
    ui->LayersTreeView->header()->setSortIndicator(0, Qt::AscendingOrder);
    ui->LayersTreeView->header()->resizeSection(0, Col0Width);
    ui->LayersTreeView->header()->resizeSection(1, Col1Width);

    mTypesProxyModel.setSourceModel(&mTypesModel);
    ui->TypesTreeView->setModel(&mTypesProxyModel);
    ui->TypesTreeView->header()->setSortIndicator(0, Qt::AscendingOrder);
    ui->TypesTreeView->header()->resizeSection(0, Col0Width);
    ui->TypesTreeView->header()->resizeSection(1, Col1Width);

    ui->LayersTreeView->expand(mLayersProxyModel.index(0, 0));
    ui->TypesTreeView->expand(mTypesProxyModel.index(0, 0));

    ui->ButtonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

    connect(ui->TabWidget, SIGNAL(currentChanged(int)), this, SLOT(OnTabChanged(int)));
    connect(ui->LayersTreeView, SIGNAL(clicked(QModelIndex)), this, SLOT(OnTreeClicked(QModelIndex)));
    connect(ui->LayersTreeView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(OnTreeDoubleClicked(QModelIndex)));
    connect(ui->TypesTreeView, SIGNAL(clicked(QModelIndex)), this, SLOT(OnTreeClicked(QModelIndex)));
    connect(ui->TypesTreeView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(OnTreeDoubleClicked(QModelIndex)));
}

CSelectInstanceDialog::~CSelectInstanceDialog()
{
    delete ui;
}

CScriptObject* CSelectInstanceDialog::SelectedInstance() const
{
    return (ui->TabWidget->currentIndex() == 0 ? mpLayersInst : mpTypesInst);
}

// ************ PUBLIC SLOTS ************
void CSelectInstanceDialog::OnTabChanged(int NewTabIndex)
{
    if (NewTabIndex == 0)
        mValidSelection = (mpLayersInst != nullptr);
    else
        mValidSelection = (mpTypesInst != nullptr);

    ui->ButtonBox->button(QDialogButtonBox::Ok)->setEnabled(mValidSelection);
}

void CSelectInstanceDialog::OnTreeClicked(QModelIndex Index)
{
    int TabIndex = ui->TabWidget->currentIndex();

    if (TabIndex == 0)
    {
        QModelIndex SourceIndex = mLayersProxyModel.mapToSource(Index);
        mpLayersInst = mLayersModel.IndexObject(SourceIndex);
        mValidSelection = (mpLayersInst != nullptr);
    }

    else
    {
        QModelIndex SourceIndex = mTypesProxyModel.mapToSource(Index);
        mpTypesInst = mTypesModel.IndexObject(SourceIndex);
        mValidSelection = (mpTypesInst != nullptr);
    }

    ui->ButtonBox->button(QDialogButtonBox::Ok)->setEnabled(mValidSelection);
}

void CSelectInstanceDialog::OnTreeDoubleClicked(QModelIndex /*Index*/)
{
    // Instance selection was handled in OnTreeClicked on the first click.
    if (mValidSelection)
        close();
}
