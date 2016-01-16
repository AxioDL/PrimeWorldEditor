#include "CPoiMapEditDialog.h"
#include "ui_CPoiMapEditDialog.h"
#include "CWorldEditor.h"
#include "Editor/UICommon.h"

#include <Core/Resource/CScan.h>
#include <Core/Resource/Cooker/CPoiToWorldCooker.h>
#include <Core/ScriptExtra/CPointOfInterestExtra.h>

#include <QMouseEvent>
#include <QMessageBox>

const CColor CPoiMapEditDialog::skNormalColor(0.137255f, 0.184314f, 0.776471f, 0.5f);
const CColor CPoiMapEditDialog::skImportantColor(0.721569f, 0.066667f, 0.066667f, 0.5f);

CPoiMapEditDialog::CPoiMapEditDialog(CWorldEditor *pEditor, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::CPoiMapEditDialog)
    , mpEditor(pEditor)
    , mSourceModel(pEditor, this)
    , mHighlightMode(eHighlightSelected)
    , mPickType(eNotPicking)
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
    connect(ui->AddMeshButton, SIGNAL(clicked()), this, SLOT(PickButtonClicked()));
    connect(ui->RemoveMeshButton, SIGNAL(clicked()), this, SLOT(PickButtonClicked()));
    connect(ui->ButtonBox->button(QDialogButtonBox::Ok), SIGNAL(clicked()), this, SLOT(close()));
    connect(ui->ButtonBox->button(QDialogButtonBox::Cancel), SIGNAL(clicked()), this, SLOT(close()));
    connect(ui->ButtonBox->button(QDialogButtonBox::Save), SIGNAL(clicked()), this, SLOT(Save()));
}

CPoiMapEditDialog::~CPoiMapEditDialog()
{
    delete ui;

    // Clear model tints
    if (mHighlightMode != eHighlightNone)
        SetHighlightNone();

    // Stop picking
    if (mPickType != eNotPicking)
        StopPicking();
}

void CPoiMapEditDialog::closeEvent(QCloseEvent* /*pEvent*/)
{
    if (mPickType != eNotPicking)
        mpEditor->ExitPickMode();

    emit Closed();
}

void CPoiMapEditDialog::HighlightPoiModels(const QModelIndex& rkIndex)
{
    // Get POI and models
    QModelIndex SourceIndex = mModel.mapToSource(rkIndex);
    const QList<CModelNode*>& rkModels = mSourceModel.GetPoiMeshList(SourceIndex);
    bool Important = IsImportant(SourceIndex);

    // Highlight the meshes
    for (int iMdl = 0; iMdl < rkModels.size(); iMdl++)
    {
        rkModels[iMdl]->SetScanOverlayEnabled(true);
        rkModels[iMdl]->SetScanOverlayColor(Important ? skImportantColor : skNormalColor);
    }
}

void CPoiMapEditDialog::UnhighlightPoiModels(const QModelIndex& rkIndex)
{
    QModelIndex SourceIndex = mModel.mapToSource(rkIndex);
    const QList<CModelNode*>& rkModels = mSourceModel.GetPoiMeshList(SourceIndex);

    for (int iMdl = 0; iMdl < rkModels.size(); iMdl++)
        rkModels[iMdl]->SetScanOverlayEnabled(false);
}

void CPoiMapEditDialog::HighlightModel(const QModelIndex& rkIndex, CModelNode *pNode)
{
    bool Important = IsImportant(rkIndex);
    pNode->SetScanOverlayEnabled(true);
    pNode->SetScanOverlayColor(Important ? skImportantColor : skNormalColor);
}

void CPoiMapEditDialog::UnhighlightModel(CModelNode *pNode)
{
    pNode->SetScanOverlayEnabled(false);
}

bool CPoiMapEditDialog::IsImportant(const QModelIndex& rkIndex)
{
    CScriptNode *pPOI = mSourceModel.PoiNodePointer(rkIndex);

    bool Important = false;
    TResPtr<CScan> pScan = static_cast<CPointOfInterestExtra*>(pPOI->Extra())->GetScan();

    if (pScan)
        Important = pScan->IsImportant();

    return Important;
}

void CPoiMapEditDialog::Save()
{
    CPoiToWorld *pPoiToWorld = mpEditor->ActiveArea()->GetPoiToWorldMap();

    TString FileName = pPoiToWorld->FullSource();
    CFileOutStream Out(FileName.ToStdString(), IOUtil::eBigEndian);

    if (Out.IsValid())
    {
        CPoiToWorldCooker::WriteEGMC(pPoiToWorld, Out);
        QMessageBox::information(this, "Saved", QString("Saved to %1!").arg(TO_QSTRING(pPoiToWorld->Source())));
    }
    else
        QMessageBox::warning(this, "Error", "Couldn't save EGMC; unable to open output file");
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

void CPoiMapEditDialog::PickButtonClicked()
{
    QPushButton *pButton = qobject_cast<QPushButton*>(sender());

    if (!pButton->isChecked())
        mpEditor->ExitPickMode();

    else
    {
        mpEditor->EnterPickMode(eModelNode, false, false);
        connect(mpEditor, SIGNAL(PickModeExited()), this, SLOT(StopPicking()));
        connect(mpEditor, SIGNAL(PickModeClick(CSceneNode*,QMouseEvent*)), this, SLOT(OnNodePicked(CSceneNode*,QMouseEvent*)));
        pButton->setChecked(true);

        if (pButton == ui->AddMeshButton)
        {
            mPickType = eAddMeshes;
            ui->RemoveMeshButton->setChecked(false);
        }

        else if (pButton == ui->RemoveMeshButton)
        {
            mPickType = eRemoveMeshes;
            ui->AddMeshButton->setChecked(false);
        }
    }
}

void CPoiMapEditDialog::StopPicking()
{
    ui->AddMeshButton->setChecked(false);
    ui->RemoveMeshButton->setChecked(false);
    mPickType = eNotPicking;

    disconnect(mpEditor, 0, this, 0);
}

void CPoiMapEditDialog::OnNodePicked(CSceneNode *pNode, QMouseEvent* pEvent)
{
    // Check for valid selection
    QModelIndexList Indices = ui->ListView->selectionModel()->selectedRows();
    if (Indices.isEmpty()) return;

    // Map selection to source model
    QModelIndexList SourceIndices;
    for (auto it = Indices.begin(); it != Indices.end(); it++)
        SourceIndices << mModel.mapToSource(*it);

    // If alt is pressed, invert the pick mode
    CModelNode *pModel = static_cast<CModelNode*>(pNode);
    bool AltPressed = (pEvent->modifiers() & Qt::AltModifier) != 0;

    EPickType PickType;
    if (!AltPressed)                    PickType = mPickType;
    else if (mPickType == eAddMeshes)   PickType = eRemoveMeshes;
    else                                PickType = eAddMeshes;

    // Add meshes
    if (PickType == eAddMeshes)
    {
        for (auto it = SourceIndices.begin(); it != SourceIndices.end(); it++)
            mSourceModel.AddMapping(*it, pModel);

        if (mHighlightMode != eHighlightNone)
            HighlightModel(SourceIndices.front(), pModel);
    }

    // Remove meshes
    else if (PickType == eRemoveMeshes)
    {
        for (auto it = SourceIndices.begin(); it != SourceIndices.end(); it++)
            mSourceModel.RemoveMapping(*it, pModel);

        if (mHighlightMode != eHighlightNone)
            UnhighlightModel(pModel);
    }
}
