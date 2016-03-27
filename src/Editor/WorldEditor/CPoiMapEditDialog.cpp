#include "CPoiMapEditDialog.h"
#include "ui_CPoiMapEditDialog.h"
#include "CWorldEditor.h"
#include "Editor/UICommon.h"

#include <Core/Resource/CScan.h>
#include <Core/Resource/Cooker/CPoiToWorldCooker.h>
#include <Core/Resource/Script/CMasterTemplate.h>
#include <Core/ScriptExtra/CPointOfInterestExtra.h>

#include <QMouseEvent>
#include <QMessageBox>

const CColor CPoiMapEditDialog::skNormalColor(0.137255f, 0.184314f, 0.776471f, 0.5f);
const CColor CPoiMapEditDialog::skImportantColor(0.721569f, 0.066667f, 0.066667f, 0.5f);
const CColor CPoiMapEditDialog::skHoverColor(0.047059f, 0.2f, 0.003922f, 0.5f);

CPoiMapEditDialog::CPoiMapEditDialog(CWorldEditor *pEditor, QWidget *pParent)
    : QMainWindow(pParent)
    , ui(new Ui::CPoiMapEditDialog)
    , mpEditor(pEditor)
    , mSourceModel(pEditor, this)
    , mHighlightMode(eHighlightSelected)
    , mPickType(eNotPicking)
    , mpHoverModel(nullptr)
{
    mModel.setSourceModel(&mSourceModel);
    mModel.sort(0);

    ui->setupUi(this);
    ui->ListView->setModel(&mModel);
    ui->ListView->selectionModel()->select(mModel.index(0,0), QItemSelectionModel::Select | QItemSelectionModel::Current);

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
    connect(ui->MapMeshesButton, SIGNAL(clicked()), this, SLOT(OnPickButtonClicked()));
    connect(ui->UnmapMeshesButton, SIGNAL(clicked()), this, SLOT(OnPickButtonClicked()));
    connect(ui->UnmapAllButton, SIGNAL(clicked()), this, SLOT(OnUnmapAllPressed()));
    connect(ui->AddPoiFromViewportButton, SIGNAL(clicked()), this, SLOT(OnPickButtonClicked()));
    connect(ui->AddPoiFromInstanceListButton, SIGNAL(clicked()), this, SLOT(OnInstanceListButtonClicked()));
    connect(ui->RemovePoiButton, SIGNAL(clicked()), this, SLOT(OnRemovePoiButtonClicked()));
    connect(ui->ButtonBox->button(QDialogButtonBox::Close), SIGNAL(clicked()), this, SLOT(close()));
    connect(ui->ButtonBox->button(QDialogButtonBox::Save), SIGNAL(clicked()), this, SLOT(Save()));
}

CPoiMapEditDialog::~CPoiMapEditDialog()
{
    // Clear model tints
    if (mHighlightMode != eHighlightNone)
        SetHighlightNone();

    // Stop picking
    if (mPickType != eNotPicking)
        StopPicking();

    delete ui;
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
        RevertModelOverlay(rkModels[iMdl]);
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

void CPoiMapEditDialog::RevertModelOverlay(CModelNode *pModel)
{
    if (pModel)
    {
        if (mHighlightMode == eHighlightAll)
        {
            // Prioritize the selected POI over others.
            QModelIndex Selected = GetSelectedRow();

            if (mSourceModel.IsModelMapped(Selected, pModel))
                HighlightModel(Selected, pModel);

            // If it's not mapped to the selected POI, then check whether it's mapped to any others.
            else
            {
                for (int iRow = 0; iRow < mSourceModel.rowCount(QModelIndex()); iRow++)
                {
                    QModelIndex Index = mSourceModel.index(iRow, 0);

                    if (mSourceModel.IsModelMapped(Index, pModel))
                    {
                        HighlightModel(Index, pModel);
                        return;
                    }
                }

                UnhighlightModel(pModel);
            }
        }

        else if (mHighlightMode == eHighlightSelected)
        {
            QModelIndex Index = GetSelectedRow();

            if (mSourceModel.IsModelMapped(Index, pModel))
                HighlightModel(Index, pModel);
            else
                UnhighlightModel(pModel);
        }

        else
            UnhighlightModel(pModel);
    }
}

CPoiMapEditDialog::EPickType CPoiMapEditDialog::GetRealPickType(bool AltPressed) const
{
    if (!AltPressed) return mPickType;
    if (mPickType == eAddMeshes) return eRemoveMeshes;
    return eAddMeshes;
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

QModelIndex CPoiMapEditDialog::GetSelectedRow() const
{
    QModelIndexList Indices = ui->ListView->selectionModel()->selectedRows();
    return ( Indices.isEmpty() ? QModelIndex() : mModel.mapToSource(Indices.front()) );
}

void CPoiMapEditDialog::Save()
{
    CPoiToWorld *pPoiToWorld = mpEditor->ActiveArea()->PoiToWorldMap();

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
    mHighlightMode = eHighlightSelected;

    const QItemSelection kSelection = ui->ListView->selectionModel()->selection();
    QList<QModelIndex> SelectedIndices;
    QList<QModelIndex> UnselectedIndices;

    for (int iRow = 0; iRow < mModel.rowCount(QModelIndex()); iRow++)
    {
        QModelIndex Index = mModel.index(iRow, 0);

        if (kSelection.contains(Index))
            SelectedIndices << Index;
        else
            UnselectedIndices << Index;
    }

    for (int iIdx = 0; iIdx < UnselectedIndices.size(); iIdx++)
        UnhighlightPoiModels(UnselectedIndices[iIdx]);
    for (int iIdx = 0; iIdx < SelectedIndices.size(); iIdx++)
        HighlightPoiModels(SelectedIndices[iIdx]);
}

void CPoiMapEditDialog::SetHighlightAll()
{
    mHighlightMode = eHighlightAll;

    for (int iRow = 0; iRow < mModel.rowCount(QModelIndex()); iRow++)
        HighlightPoiModels(mModel.index(iRow, 0));

    // Call HighlightPoiModels again on the selected index to prioritize it over the non-selected POIs.
    if (ui->ListView->selectionModel()->hasSelection())
        HighlightPoiModels(ui->ListView->selectionModel()->selectedRows().front());

    if (mpHoverModel)
        HighlightModel(GetSelectedRow(), mpHoverModel);
}

void CPoiMapEditDialog::SetHighlightNone()
{
    mHighlightMode = eHighlightNone;

    for (int iRow = 0; iRow < mModel.rowCount(QModelIndex()); iRow++)
        UnhighlightPoiModels(mModel.index(iRow, 0));

    if (mpHoverModel)
        UnhighlightModel(mpHoverModel);
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

void CPoiMapEditDialog::OnUnmapAllPressed()
{
    QModelIndex Index = GetSelectedRow();
    QList<CModelNode*> ModelList = mSourceModel.GetPoiMeshList(Index);

    foreach (CModelNode *pModel, ModelList)
    {
        mSourceModel.RemoveMapping(Index, pModel);
        RevertModelOverlay(pModel);
    }
}

void CPoiMapEditDialog::OnPickButtonClicked()
{
    QPushButton *pButton = qobject_cast<QPushButton*>(sender());

    if (pButton == ui->AddPoiFromViewportButton)
    {
        mpEditor->EnterPickMode(eScriptNode, true, false, false);
        connect(mpEditor, SIGNAL(PickModeExited()), this, SLOT(StopPicking()));
        connect(mpEditor, SIGNAL(PickModeClick(SRayIntersection,QMouseEvent*)), this, SLOT(OnPoiPicked(SRayIntersection,QMouseEvent*)));

        pButton->setChecked(true);
        ui->MapMeshesButton->setChecked(false);
        ui->UnmapMeshesButton->setChecked(false);

        mPickType = eAddPOIs;
    }

    else
    {
        if (!pButton->isChecked())
            mpEditor->ExitPickMode();

        else
        {
            mpEditor->EnterPickMode(eModelNode, false, false, true);
            connect(mpEditor, SIGNAL(PickModeExited()), this, SLOT(StopPicking()));
            connect(mpEditor, SIGNAL(PickModeHoverChanged(SRayIntersection,QMouseEvent*)), this, SLOT(OnModelHover(SRayIntersection,QMouseEvent*)));
            pButton->setChecked(true);

            if (pButton == ui->MapMeshesButton)
            {
                mPickType = eAddMeshes;
                ui->UnmapMeshesButton->setChecked(false);
            }

            else if (pButton == ui->UnmapMeshesButton)
            {
                mPickType = eRemoveMeshes;
                ui->MapMeshesButton->setChecked(false);
            }
        }
    }
}

void CPoiMapEditDialog::StopPicking()
{
    ui->MapMeshesButton->setChecked(false);
    ui->UnmapMeshesButton->setChecked(false);
    ui->AddPoiFromViewportButton->setChecked(false);
    mPickType = eNotPicking;

    RevertModelOverlay(mpHoverModel);
    mpHoverModel = nullptr;

    disconnect(mpEditor, 0, this, 0);
}

void CPoiMapEditDialog::OnInstanceListButtonClicked()
{
    EGame Game = mpEditor->ActiveArea()->Version();
    CScriptTemplate *pPoiTemplate = CMasterTemplate::MasterForGame(Game)->TemplateByID("POIN");

    CPoiListDialog Dialog(pPoiTemplate, &mSourceModel, mpEditor->Scene(), this);
    Dialog.exec();

    const QList<CScriptNode*>& rkSelection = Dialog.Selection();

    if (!rkSelection.empty())
    {
        foreach (CScriptNode *pNode, rkSelection)
            mSourceModel.AddPOI(pNode);

        mModel.sort(0);
    }
}

void CPoiMapEditDialog::OnRemovePoiButtonClicked()
{
    if (ui->ListView->selectionModel()->hasSelection())
    {
        QModelIndex Index = ui->ListView->selectionModel()->selectedRows().front();
        UnhighlightPoiModels(Index);
        Index = mModel.mapToSource(Index);
        mSourceModel.RemovePOI(Index);
    }
}

void CPoiMapEditDialog::OnPoiPicked(const SRayIntersection& rkIntersect, QMouseEvent *pEvent)
{
    CScriptNode *pPOI = static_cast<CScriptNode*>(rkIntersect.pNode);
    if (pPOI->Instance()->ObjectTypeID() != CFourCC("POIN").ToLong()) return;

    mSourceModel.AddPOI(pPOI);
    mModel.sort(0);

    // Exit pick mode unless the user is holding the Ctrl key
    if (!(pEvent->modifiers() & Qt::ControlModifier))
        mpEditor->ExitPickMode();
}

void CPoiMapEditDialog::OnModelPicked(const SRayIntersection& rkRayIntersect, QMouseEvent* pEvent)
{
    if (!rkRayIntersect.pNode) return;

    // Check for valid selection
    QModelIndexList Indices = ui->ListView->selectionModel()->selectedRows();
    if (Indices.isEmpty()) return;

    // Map selection to source model
    QModelIndexList SourceIndices;
    for (auto it = Indices.begin(); it != Indices.end(); it++)
        SourceIndices << mModel.mapToSource(*it);

    // If alt is pressed, invert the pick mode
    CModelNode *pModel = static_cast<CModelNode*>(rkRayIntersect.pNode);

    bool AltPressed = (pEvent->modifiers() & Qt::AltModifier) != 0;
    EPickType PickType = GetRealPickType(AltPressed);

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
            RevertModelOverlay(mpHoverModel);
        else
            UnhighlightModel(pModel);
    }
}

void CPoiMapEditDialog::OnModelHover(const SRayIntersection& rkIntersect, QMouseEvent *pEvent)
{
    // Restore old hover model to correct overlay color, and set new hover model
    if (mpHoverModel)
        RevertModelOverlay(mpHoverModel);

    mpHoverModel = static_cast<CModelNode*>(rkIntersect.pNode);

    // If the left mouse button is pressed, treat this as a click.
    if (pEvent->buttons() & Qt::LeftButton)
        OnModelPicked(rkIntersect, pEvent);

    // Otherwise, process as a mouseover
    else
    {
        QModelIndex Index = GetSelectedRow();

        // Process new hover model
        if (mpHoverModel)
        {
            bool AltPressed = (pEvent->modifiers() & Qt::AltModifier) != 0;
            EPickType PickType = GetRealPickType(AltPressed);

            if ( ((PickType == eAddMeshes) && !mSourceModel.IsModelMapped(Index, mpHoverModel)) ||
                 ((PickType == eRemoveMeshes) && mSourceModel.IsModelMapped(Index, mpHoverModel)) )
            {
                mpHoverModel->SetScanOverlayEnabled(true);
                mpHoverModel->SetScanOverlayColor(skHoverColor);
            }
        }
    }
}
