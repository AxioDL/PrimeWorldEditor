#include "CPoiMapEditDialog.h"
#include "ui_CPoiMapEditDialog.h"
#include "CWorldEditor.h"
#include <Core/Resource/CScan.h>
#include <Core/ScriptExtra/CPointOfInterestExtra.h>

CPoiMapEditDialog::CPoiMapEditDialog(CWorldEditor *pEditor, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::CPoiMapEditDialog)
    , mpEditor(pEditor)
    , mSourceModel(pEditor, this)
    , mHighlightMode(eHighlightSelected)
{
    mModel.setSourceModel(&mSourceModel);
    mModel.sort(0);

    ui->setupUi(this);
    ui->ListView->setModel(&mModel);

    QActionGroup *pGroup = new QActionGroup(this);
    pGroup->addAction(ui->ActionHighlightSelected);
    pGroup->addAction(ui->ActionHighlightAll);
    pGroup->addAction(ui->ActionHighlightNone);
    SetHighlightSelected();

    connect(ui->ActionHighlightSelected, SIGNAL(triggered()), this, SLOT(SetHighlightSelected()));
    connect(ui->ActionHighlightAll, SIGNAL(triggered()), this, SLOT(SetHighlightAll()));
    connect(ui->ActionHighlightNone, SIGNAL(triggered()), this, SLOT(SetHighlightNone()));
    connect(ui->ListView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this, SLOT(OnSelectionChanged(QItemSelection,QItemSelection)));
    connect(ui->ListView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(OnItemDoubleClick(QModelIndex)));
    connect(ui->ButtonBox, SIGNAL(accepted()), this, SLOT(close()));
    connect(ui->ButtonBox, SIGNAL(rejected()), this, SLOT(close()));
}

CPoiMapEditDialog::~CPoiMapEditDialog()
{
    delete ui;

    // Clear model tints
    if (mHighlightMode != eHighlightNone)
        SetHighlightNone();
}

void CPoiMapEditDialog::HighlightPoiModels(const QModelIndex& rkIndex)
{
    // Get POI and models
    QModelIndex SourceIndex = mModel.mapToSource(rkIndex);
    CScriptNode *pPOI = mSourceModel.PoiNodePointer(SourceIndex);
    const QList<CModelNode*>& rkModels = mSourceModel.GetPoiMeshList(pPOI);

    // Check whether this is an important scan
    bool IsImportant = false;
    TResPtr<CScan> pScan = static_cast<CPointOfInterestExtra*>(pPOI->Extra())->GetScan();

    if (pScan)
        IsImportant = pScan->IsImportant();

    // Highlight the meshes
    for (int iMdl = 0; iMdl < rkModels.size(); iMdl++)
        rkModels[iMdl]->SetTintColor(IsImportant ? CColor::skRed : CColor::skBlue);
}

void CPoiMapEditDialog::UnhighlightPoiModels(const QModelIndex& rkIndex)
{
    QModelIndex SourceIndex = mModel.mapToSource(rkIndex);
    const QList<CModelNode*>& rkModels = mSourceModel.GetPoiMeshList(SourceIndex);

    for (int iMdl = 0; iMdl < rkModels.size(); iMdl++)
        rkModels[iMdl]->ClearTintColor();
}

void CPoiMapEditDialog::SetHighlightSelected()
{
    const QItemSelection kSelection = ui->ListView->selectionModel()->selection();

    for (int iRow = 0; iRow < mModel.rowCount(QModelIndex()); iRow++)
    {
        QModelIndex Index = mModel.index(iRow, 0);

        if (kSelection.contains(Index))
            HighlightPoiModels(Index);
        else
            UnhighlightPoiModels(Index);
    }

    mHighlightMode = eHighlightSelected;
}

void CPoiMapEditDialog::SetHighlightAll()
{
    for (int iRow = 0; iRow < mModel.rowCount(QModelIndex()); iRow++)
        HighlightPoiModels(mModel.index(iRow, 0));

    mHighlightMode = eHighlightAll;
}

void CPoiMapEditDialog::SetHighlightNone()
{
    for (int iRow = 0; iRow < mModel.rowCount(QModelIndex()); iRow++)
        UnhighlightPoiModels(mModel.index(iRow, 0));

    mHighlightMode = eHighlightNone;
}

void CPoiMapEditDialog::OnSelectionChanged(const QItemSelection& rkSelected, const QItemSelection& rkDeselected)
{
    if (mHighlightMode == eHighlightSelected)
    {
        // Clear highlight on deselected models
        QModelIndexList DeselectedIndices = rkDeselected.indexes();

        for (int iIdx = 0; iIdx < DeselectedIndices.size(); iIdx++)
            UnhighlightPoiModels(DeselectedIndices[iIdx]);

        // Highlight newly selected models
        QModelIndexList SelectedIndices = rkSelected.indexes();

        for (int iIdx = 0; iIdx < SelectedIndices.size(); iIdx++)
            HighlightPoiModels(SelectedIndices[iIdx]);
    }
}

void CPoiMapEditDialog::OnItemDoubleClick(QModelIndex Index)
{
    QModelIndex SourceIndex = mModel.mapToSource(Index);
    CScriptNode *pPOI = mSourceModel.PoiNodePointer(SourceIndex);
    mpEditor->ClearAndSelectNode(pPOI);
}
