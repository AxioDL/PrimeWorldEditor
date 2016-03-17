#include "CWorldEditor.h"
#include "ui_CWorldEditor.h"
#include "CConfirmUnlinkDialog.h"
#include "CLayerEditor.h"
#include "CTemplateMimeData.h"
#include "WModifyTab.h"
#include "WInstancesTab.h"

#include "Editor/CBasicViewport.h"
#include "Editor/CSelectionIterator.h"
#include "Editor/UICommon.h"
#include "Editor/PropertyEdit/CPropertyView.h"
#include "Editor/Widgets/WDraggableSpinBox.h"
#include "Editor/Widgets/WVectorEditor.h"
#include "Editor/Undo/UndoCommands.h"

#include <Core/Render/CDrawUtil.h>
#include <Core/Resource/Cooker/CAreaCooker.h>
#include <Core/Scene/CSceneIterator.h>
#include <Common/Log.h>

#include <iostream>
#include <QComboBox>
#include <QFontMetrics>
#include <QMessageBox>
#include <QOpenGLContext>

CWorldEditor::CWorldEditor(QWidget *parent)
    : INodeEditor(parent)
    , ui(new Ui::CWorldEditor)
    , mpArea(nullptr)
    , mpWorld(nullptr)
    , mpLinkDialog(new CLinkDialog(this, this))
    , mpPoiDialog(nullptr)
    , mIsMakingLink(false)
    , mpNewLinkSender(nullptr)
    , mpNewLinkReceiver(nullptr)
{
    Log::Write("Creating World Editor");
    ui->setupUi(this);

    mpSelection->SetAllowedNodeTypes(eScriptNode | eLightNode);

    // Start refresh timer
    connect(&mRefreshTimer, SIGNAL(timeout()), this, SLOT(RefreshViewport()));
    mRefreshTimer.start(0);

    // Initialize splitter
    QList<int> SplitterSizes;
    SplitterSizes << width() * 0.775 << width() * 0.225;
    ui->splitter->setSizes(SplitterSizes);

    // Initialize UI stuff
    ui->MainViewport->SetScene(this, &mScene);
    ui->CreateTabContents->SetEditor(this);
    ui->ModifyTabContents->SetEditor(this);
    ui->InstancesTabContents->SetEditor(this, &mScene);
    ui->TransformSpinBox->SetOrientation(Qt::Horizontal);
    ui->TransformSpinBox->layout()->setContentsMargins(0,0,0,0);
    ui->CamSpeedSpinBox->SetDefaultValue(1.0);

    mpTransformCombo->setMinimumWidth(75);
    ui->MainToolBar->addActions(mGizmoActions);
    ui->MainToolBar->addWidget(mpTransformCombo);
    ui->menuEdit->insertActions(ui->ActionSelectAll, mUndoActions);
    ui->menuEdit->insertSeparator(ui->ActionSelectAll);

    // Initialize actions
    addAction(ui->ActionIncrementGizmo);
    addAction(ui->ActionDecrementGizmo);
    addAction(ui->ActionDelete);

    QAction *pToolBarUndo = mUndoStack.createUndoAction(this);
    pToolBarUndo->setIcon(QIcon(":/icons/Undo.png"));
    ui->MainToolBar->insertAction(ui->ActionLink, pToolBarUndo);

    QAction *pToolBarRedo = mUndoStack.createRedoAction(this);
    pToolBarRedo->setIcon(QIcon(":/icons/Redo.png"));
    ui->MainToolBar->insertAction(ui->ActionLink, pToolBarRedo);
    ui->MainToolBar->insertSeparator(ui->ActionLink);

    // Connect signals and slots
    connect(ui->MainViewport, SIGNAL(ViewportClick(SRayIntersection,QMouseEvent*)), this, SLOT(OnViewportClick(SRayIntersection,QMouseEvent*)));
    connect(ui->MainViewport, SIGNAL(InputProcessed(SRayIntersection,QMouseEvent*)), this, SLOT(OnViewportInputProcessed(SRayIntersection,QMouseEvent*)));
    connect(ui->MainViewport, SIGNAL(InputProcessed(SRayIntersection,QMouseEvent*)), this, SLOT(UpdateGizmoUI()) );
    connect(ui->MainViewport, SIGNAL(InputProcessed(SRayIntersection,QMouseEvent*)), this, SLOT(UpdateStatusBar()) );
    connect(ui->MainViewport, SIGNAL(InputProcessed(SRayIntersection,QMouseEvent*)), this, SLOT(UpdateCursor()) );
    connect(ui->MainViewport, SIGNAL(GizmoMoved()), this, SLOT(OnGizmoMoved()));
    connect(ui->MainViewport, SIGNAL(CameraOrbit()), this, SLOT(UpdateCameraOrbit()));
    connect(this, SIGNAL(SelectionModified()), this, SLOT(UpdateCameraOrbit()));
    connect(this, SIGNAL(SelectionTransformed()), this, SLOT(UpdateCameraOrbit()));
    connect(this, SIGNAL(PickModeEntered(QCursor)), this, SLOT(OnPickModeEnter(QCursor)));
    connect(this, SIGNAL(PickModeExited()), this, SLOT(OnPickModeExit()));
    connect(ui->TransformSpinBox, SIGNAL(ValueChanged(CVector3f)), this, SLOT(OnTransformSpinBoxModified(CVector3f)));
    connect(ui->TransformSpinBox, SIGNAL(EditingDone(CVector3f)), this, SLOT(OnTransformSpinBoxEdited(CVector3f)));
    connect(ui->CamSpeedSpinBox, SIGNAL(valueChanged(double)), this, SLOT(OnCameraSpeedChange(double)));
    connect(ui->ActionLink, SIGNAL(toggled(bool)), this, SLOT(OnLinkButtonToggled(bool)));
    connect(ui->ActionUnlink, SIGNAL(triggered()), this, SLOT(OnUnlinkClicked()));
    connect(ui->ActionDelete, SIGNAL(triggered()), this, SLOT(DeleteSelection()));
    connect(&mUndoStack, SIGNAL(indexChanged(int)), this, SLOT(OnUndoStackIndexChanged()));

    connect(ui->ActionSave, SIGNAL(triggered()), this, SLOT(Save()));

    ui->CreateTabEditorProperties->SyncToEditor(this);
    ui->ModifyTabEditorProperties->SyncToEditor(this);
    ui->InstancesTabEditorProperties->SyncToEditor(this);
    ui->MainViewport->setAcceptDrops(true);
}

CWorldEditor::~CWorldEditor()
{
    delete ui;
}

void CWorldEditor::closeEvent(QCloseEvent *pEvent)
{
    bool ShouldClose = CheckUnsavedChanges();

    if (ShouldClose)
    {
        mUndoStack.clear();
        mpLinkDialog->close();

        if (mpPoiDialog)
            mpPoiDialog->close();

        emit Closed();
    }
    else
    {
        pEvent->ignore();
    }
}

void CWorldEditor::SetArea(CWorld *pWorld, CGameArea *pArea)
{
    ExitPickMode();
    ui->MainViewport->ResetHover();
    ClearSelection();
    ui->ModifyTabContents->ClearUI();
    ui->InstancesTabContents->SetMaster(nullptr);
    ui->InstancesTabContents->SetArea(pArea);
    mUndoStack.clear();

    if (mpPoiDialog)
    {
        delete mpPoiDialog;
        mpPoiDialog = nullptr;
    }

    // Clear old area - hack until better world/area loader is implemented
    if ((mpArea) && (pArea != mpArea))
        mpArea->ClearScriptLayers();

    // Load new area
    mpArea = pArea;
    mpWorld = pWorld;
    mScene.SetActiveWorld(pWorld);
    mScene.SetActiveArea(pArea);

    // Snap camera to new area
    CCamera *pCamera = &ui->MainViewport->Camera();

    if (pCamera->MoveMode() == eFreeCamera)
    {
        CTransform4f AreaTransform = pArea->GetTransform();
        CVector3f AreaPosition(AreaTransform[0][3], AreaTransform[1][3], AreaTransform[2][3]);
        pCamera->Snap(AreaPosition);
    }

    UpdateCameraOrbit();

    // Default bloom to Fake Bloom for Metroid Prime 3; disable for other games
    bool AllowBloom = (mpWorld->Version() == eCorruptionProto || mpWorld->Version() == eCorruption);
    AllowBloom ? on_ActionFakeBloom_triggered() : on_ActionNoBloom_triggered();
    ui->menuBloom->setEnabled(AllowBloom);

    // Disable EGMC editing for Prime 1 and DKCR
    bool AllowEGMC = ( (mpWorld->Version() >= eEchoesDemo) && (mpWorld->Version() <= eCorruption) );
    ui->ActionEditPoiToWorldMap->setEnabled(AllowEGMC);

    // Set up sidebar tabs
    CMasterTemplate *pMaster = CMasterTemplate::GetMasterForGame(mpArea->Version());
    ui->CreateTabContents->SetMaster(pMaster);
    ui->InstancesTabContents->SetMaster(pMaster);

    // Set up dialogs
    mpLinkDialog->SetMaster(pMaster);

    // Set window title
    CStringTable *pWorldNameTable = mpWorld->GetWorldName();
    TWideString WorldName = pWorldNameTable ? pWorldNameTable->GetString("ENGL", 0) : "[Untitled World]";

    if (CurrentGame() < eReturns)
    {
        CStringTable *pAreaNameTable = mpWorld->GetAreaName(mpArea->WorldIndex());
        TWideString AreaName = pAreaNameTable ? pAreaNameTable->GetString("ENGL", 0) : (TWideString("!") + mpWorld->GetAreaInternalName(mpArea->WorldIndex()).ToUTF16());

        if (AreaName.IsEmpty())
            AreaName = "[Untitled Area]";

        setWindowTitle(QString("Prime World Editor - %1 - %2[*]").arg(TO_QSTRING(WorldName)).arg(TO_QSTRING(AreaName)));
        Log::Write("Loaded area: " + mpArea->Source() + " (" + TO_TSTRING(TO_QSTRING(AreaName)) + ")");
    }

    else
    {
        QString LevelName;
        if (pWorldNameTable) LevelName = TO_QSTRING(WorldName);
        else LevelName = "!" + TO_QSTRING(mpWorld->GetAreaInternalName(mpArea->WorldIndex()));

        setWindowTitle(QString("Prime World Editor - %1[*]").arg(LevelName));
        Log::Write("Loaded level: World " + mpWorld->Source() + " / Area " + mpArea->Source() + " (" + TO_TSTRING(LevelName) + ")");
    }

    // Emit signals
    emit LayersModified();
}

bool CWorldEditor::CheckUnsavedChanges()
{
    // Check whether the user has unsaved changes, return whether it's okay to clear the scene
    bool OkToClear = !isWindowModified();

    if (!OkToClear)
    {
        int Result = QMessageBox::warning(this, "Save", "You have unsaved changes. Save?", QMessageBox::Yes, QMessageBox::No, QMessageBox::Cancel);

        if (Result == QMessageBox::Yes)
            OkToClear = Save();

        else if (Result == QMessageBox::No)
            OkToClear = true;

        else if (Result == QMessageBox::Cancel)
            OkToClear = false;
    }

    return OkToClear;
}

CSceneViewport* CWorldEditor::Viewport() const
{
    return ui->MainViewport;
}

// ************ PUBLIC SLOTS ************
void CWorldEditor::NotifyNodeAboutToBeDeleted(CSceneNode *pNode)
{
    INodeEditor::NotifyNodeAboutToBeDeleted(pNode);

    if (ui->MainViewport->HoverNode() == pNode)
        ui->MainViewport->ResetHover();
}

bool CWorldEditor::Save()
{
    TString Out = mpArea->FullSource();
    CFileOutStream MREA(Out.ToStdString(), IOUtil::eBigEndian);

    if (MREA.IsValid())
    {
        CAreaCooker::WriteCookedArea(mpArea, MREA);
        mUndoStack.setClean();
        setWindowModified(false);
        return true;
    }
    else
    {
        QMessageBox::warning(this, "Error", "Unable to save error; couldn't open output file " + TO_QSTRING(Out));
        return false;
    }
}

void CWorldEditor::OnLinksModified(const QList<CScriptObject*>& rkInstances)
{
    foreach (CScriptObject *pInstance, rkInstances)
    {
        CScriptNode *pNode = mScene.NodeForObject(pInstance);
        pNode->LinksModified();
    }

    if (!rkInstances.isEmpty())
        emit InstanceLinksModified(rkInstances);
}

void CWorldEditor::OnPropertyModified(IProperty *pProp)
{
    bool EditorProperty = false;

    if (!mpSelection->IsEmpty() && mpSelection->Front()->NodeType() == eScriptNode)
    {
        CScriptNode *pScript = static_cast<CScriptNode*>(mpSelection->Front());
        pScript->PropertyModified(pProp);

        // Check editor property
        if (pScript->Object()->IsEditorProperty(pProp))
            EditorProperty = true;

        // If this is an editor property, update other parts of the UI to reflect the new value.
        if (EditorProperty)
        {
            UpdateStatusBar();
            UpdateSelectionUI();
        }

        // If this is a model/character, then we'll treat this as a modified selection. This is to make sure the selection bounds updates.
        if (pProp->Type() == eFileProperty)
        {
            CFileTemplate *pFile = static_cast<CFileTemplate*>(pProp->Template());

            if (pFile->AcceptsExtension("CMDL") || pFile->AcceptsExtension("ANCS") || pFile->AcceptsExtension("CHAR"))
                SelectionModified();
        }
        else if (pProp->Type() == eCharacterProperty)
            SelectionModified();

        // Emit signal so other widgets can react to the property change
        emit PropertyModified(pScript->Object(), pProp);
    }
}

void CWorldEditor::SetSelectionActive(bool Active)
{
    // Gather list of selected objects that actually have Active properties
    QVector<CScriptObject*> Objects;

    for (CSelectionIterator It(mpSelection); It; ++It)
    {
        if (It->NodeType() == eScriptNode)
        {
            CScriptNode *pScript = static_cast<CScriptNode*>(*It);
            CScriptObject *pInst = pScript->Object();
            IProperty *pActive = pInst->ActiveProperty();

            if (pActive)
                Objects << pInst;
        }
    }

    if (!Objects.isEmpty())
    {
        mUndoStack.beginMacro("Toggle Active");

        foreach (CScriptObject *pInst, Objects)
        {
            IProperty *pActive = pInst->ActiveProperty();
            IPropertyValue *pOld = pActive->RawValue()->Clone();
            pInst->SetActive(Active);
            mUndoStack.push(new CEditScriptPropertyCommand(pActive, this, pOld, true));
        }

        mUndoStack.endMacro();
    }
}

void CWorldEditor::SetSelectionInstanceNames(const QString& rkNewName, bool IsDone)
{
    // todo: this only supports one node at a time because a macro prevents us from merging undo commands
    // this is fine right now because this function is only ever called with a selection of one node, but probably want to fix in the future
    if (mpSelection->Size() == 1 && mpSelection->Front()->NodeType() == eScriptNode)
    {
        CScriptNode *pNode = static_cast<CScriptNode*>(mpSelection->Front());
        CScriptObject *pInst = pNode->Object();
        IProperty *pName = pInst->InstanceNameProperty();

        if (pName)
        {
            TString NewName = TO_TSTRING(rkNewName);
            IPropertyValue *pOld = pName->RawValue()->Clone();
            pInst->SetName(NewName);
            mUndoStack.push(new CEditScriptPropertyCommand(pName, this, pOld, IsDone, "Edit Instance Name"));
        }
    }
}

void CWorldEditor::SetSelectionLayer(CScriptLayer *pLayer)
{
    QList<CScriptNode*> ScriptNodes;

    for (CSelectionIterator It(mpSelection); It; ++It)
    {
        if (It->NodeType() == eScriptNode)
            ScriptNodes << static_cast<CScriptNode*>(*It);
    }

    if (!ScriptNodes.isEmpty())
        mUndoStack.push(new CChangeLayerCommand(this, ScriptNodes, pLayer));
}

void CWorldEditor::DeleteSelection()
{
    // note: make it only happen if there is a script node selected
    CDeleteSelectionCommand *pCmd = new CDeleteSelectionCommand(this);
    mUndoStack.push(pCmd);
}

void CWorldEditor::UpdateStatusBar()
{
    // Would be cool to do more frequent status bar updates with more info. Unfortunately, this causes lag.
    QString StatusText = "";

    if (!mGizmoHovering)
    {
        if (ui->MainViewport->underMouse())
        {
            CSceneNode *pHoverNode = ui->MainViewport->HoverNode();

            if (pHoverNode && mpSelection->IsAllowedType(pHoverNode))
                StatusText = TO_QSTRING(pHoverNode->Name());
        }
    }

    if (ui->statusbar->currentMessage() != StatusText)
        ui->statusbar->showMessage(StatusText);
}

void CWorldEditor::UpdateGizmoUI()
{
    // Update transform XYZ spin boxes
    if (!ui->TransformSpinBox->IsBeingDragged())
    {
        CVector3f spinBoxValue = CVector3f::skZero;

        // If the gizmo is transforming, use the total transform amount
        // Otherwise, use the first selected node transform, or 0 if no selection
        if (mShowGizmo)
        {
            switch (mGizmo.Mode())
            {
            case CGizmo::eTranslate:
                if (mGizmoTransforming && mGizmo.HasTransformed())
                    spinBoxValue = mGizmo.TotalTranslation();
                else if (!mpSelection->IsEmpty())
                    spinBoxValue = mpSelection->Front()->AbsolutePosition();
                break;

            case CGizmo::eRotate:
                if (mGizmoTransforming && mGizmo.HasTransformed())
                    spinBoxValue = mGizmo.TotalRotation();
                else if (!mpSelection->IsEmpty())
                    spinBoxValue = mpSelection->Front()->AbsoluteRotation().ToEuler();
                break;

            case CGizmo::eScale:
                if (mGizmoTransforming && mGizmo.HasTransformed())
                    spinBoxValue = mGizmo.TotalScale();
                else if (!mpSelection->IsEmpty())
                    spinBoxValue = mpSelection->Front()->AbsoluteScale();
                break;
            }
        }
        else if (!mpSelection->IsEmpty()) spinBoxValue = mpSelection->Front()->AbsolutePosition();

        ui->TransformSpinBox->blockSignals(true);
        ui->TransformSpinBox->SetValue(spinBoxValue);
        ui->TransformSpinBox->blockSignals(false);
    }

    // Update gizmo
    if (!mGizmoTransforming)
    {
        // Set gizmo transform
        if (!mpSelection->IsEmpty())
        {
            mGizmo.SetPosition(mpSelection->Front()->AbsolutePosition());
            mGizmo.SetLocalRotation(mpSelection->Front()->AbsoluteRotation());
        }
    }
}

void CWorldEditor::UpdateSelectionUI()
{
    // Update selection info text
    QString SelectionText;

    if (mpSelection->Size() == 1)
        SelectionText = TO_QSTRING(mpSelection->Front()->Name());
    else if (mpSelection->Size() > 1)
        SelectionText = QString("%1 objects selected").arg(mpSelection->Size());

    QFontMetrics Metrics(ui->SelectionInfoLabel->font());
    SelectionText = Metrics.elidedText(SelectionText, Qt::ElideRight, ui->SelectionInfoFrame->width() - 10);

    if (ui->SelectionInfoLabel->text() != SelectionText)
        ui->SelectionInfoLabel->setText(SelectionText);

    // Update gizmo stuff
    UpdateGizmoUI();
}

void CWorldEditor::UpdateCursor()
{
    if (ui->MainViewport->IsCursorVisible() && !mPickMode)
    {
        CSceneNode *pHoverNode = ui->MainViewport->HoverNode();

        if (ui->MainViewport->IsHoveringGizmo())
            ui->MainViewport->SetCursorState(Qt::SizeAllCursor);
        else if ((pHoverNode) && mpSelection->IsAllowedType(pHoverNode))
            ui->MainViewport->SetCursorState(Qt::PointingHandCursor);
        else
            ui->MainViewport->SetCursorState(Qt::ArrowCursor);
    }
}

void CWorldEditor::UpdateNewLinkLine()
{
    // Check if there is a sender+receiver
    if (mpLinkDialog->isVisible() && mpLinkDialog->Sender() && mpLinkDialog->Receiver() && !mpLinkDialog->IsPicking())
    {
        CVector3f Start = mScene.NodeForObject(mpLinkDialog->Sender())->CenterPoint();
        CVector3f End = mScene.NodeForObject(mpLinkDialog->Receiver())->CenterPoint();
        ui->MainViewport->SetLinkLineEnabled(true);
        ui->MainViewport->SetLinkLine(Start, End);
    }

    // Otherwise check whether there's just a sender or just a receiver
    else
    {
        CScriptObject *pSender = nullptr;
        CScriptObject *pReceiver = nullptr;

        if (mpLinkDialog->isVisible())
        {
            if (mpLinkDialog->Sender() && !mpLinkDialog->IsPickingSender())
                pSender = mpLinkDialog->Sender();
            if (mpLinkDialog->Receiver() && !mpLinkDialog->IsPickingReceiver())
                pReceiver = mpLinkDialog->Receiver();
        }
        else if (mIsMakingLink && mpNewLinkSender)
            pSender = mpNewLinkSender;
        else if (ui->ModifyTabContents->IsPicking() && ui->ModifyTabContents->EditNode()->NodeType() == eScriptNode)
            pSender = static_cast<CScriptNode*>(ui->ModifyTabContents->EditNode())->Object();

        // No sender and no receiver = no line
        if (!pSender && !pReceiver)
            ui->MainViewport->SetLinkLineEnabled(false);

        // Yes sender and yes receiver = yes line
        else if (pSender && pReceiver)
        {
            ui->MainViewport->SetLinkLineEnabled(true);
            ui->MainViewport->SetLinkLine( mScene.NodeForObject(pSender)->CenterPoint(), mScene.NodeForObject(pReceiver)->CenterPoint() );
        }

        // Compensate for missing sender or missing receiver
        else
        {
            bool IsPicking = (mIsMakingLink || mpLinkDialog->IsPicking() || ui->ModifyTabContents->IsPicking());

            if (ui->MainViewport->underMouse() && !ui->MainViewport->IsMouseInputActive() && IsPicking)
            {
                CSceneNode *pHoverNode = ui->MainViewport->HoverNode();
                CScriptObject *pInst = (pSender ? pSender : pReceiver);

                CVector3f Start = mScene.NodeForObject(pInst)->CenterPoint();
                CVector3f End = (pHoverNode && pHoverNode->NodeType() == eScriptNode ? pHoverNode->CenterPoint() : ui->MainViewport->HoverPoint());
                ui->MainViewport->SetLinkLineEnabled(true);
                ui->MainViewport->SetLinkLine(Start, End);
            }

            else
                ui->MainViewport->SetLinkLineEnabled(false);
        }
    }
}

// ************ PROTECTED ************
void CWorldEditor::GizmoModeChanged(CGizmo::EGizmoMode mode)
{
    ui->TransformSpinBox->SetSingleStep( (mode == CGizmo::eRotate ? 1.0 : 0.1) );
    ui->TransformSpinBox->SetDefaultValue( (mode == CGizmo::eScale ? 1.0 : 0.0) );
}

// ************ PRIVATE SLOTS ************
void CWorldEditor::OnLinkButtonToggled(bool Enabled)
{
    if (Enabled)
    {
        EnterPickMode(eScriptNode, true, false, false);
        connect(this, SIGNAL(PickModeClick(SRayIntersection,QMouseEvent*)), this, SLOT(OnLinkClick(SRayIntersection)));
        connect(this, SIGNAL(PickModeExited()), this, SLOT(OnLinkEnd()));
        mIsMakingLink = true;
        mpNewLinkSender = nullptr;
        mpNewLinkReceiver = nullptr;
    }

    else
    {
        if (mIsMakingLink)
            ExitPickMode();
    }
}

void CWorldEditor::OnLinkClick(const SRayIntersection& rkIntersect)
{
    if (!mpNewLinkSender)
    {
        mpNewLinkSender = static_cast<CScriptNode*>(rkIntersect.pNode)->Object();
    }

    else
    {
        mpNewLinkReceiver = static_cast<CScriptNode*>(rkIntersect.pNode)->Object();
        mpLinkDialog->NewLink(mpNewLinkSender, mpNewLinkReceiver);
        mpLinkDialog->show();
        ExitPickMode();
    }
}

void CWorldEditor::OnLinkEnd()
{
    disconnect(this, SIGNAL(PickModeClick(SRayIntersection,QMouseEvent*)), this, SLOT(OnLinkClick(SRayIntersection)));
    disconnect(this, SIGNAL(PickModeExited()), this, SLOT(OnLinkEnd()));
    ui->ActionLink->setChecked(false);
    mIsMakingLink = false;
    mpNewLinkSender = nullptr;
    mpNewLinkReceiver = nullptr;
}

void CWorldEditor::OnUnlinkClicked()
{
    QList<CScriptNode*> SelectedScriptNodes;

    for (CSelectionIterator It(mpSelection); It; ++It)
    {
        if (It->NodeType() == eScriptNode)
            SelectedScriptNodes << static_cast<CScriptNode*>(*It);
    }

    if (!SelectedScriptNodes.isEmpty())
    {
        CConfirmUnlinkDialog Dialog(this);
        Dialog.exec();

        if (Dialog.UserChoice() != CConfirmUnlinkDialog::eCancel)
        {
            mUndoStack.beginMacro("Unlink");
            bool UnlinkIncoming = (Dialog.UserChoice() != CConfirmUnlinkDialog::eOutgoingOnly);
            bool UnlinkOutgoing = (Dialog.UserChoice() != CConfirmUnlinkDialog::eIncomingOnly);

            foreach (CScriptNode *pNode, SelectedScriptNodes)
            {
                CScriptObject *pInst = pNode->Object();

                if (UnlinkIncoming)
                {
                    QVector<u32> LinkIndices;
                    for (u32 iLink = 0; iLink < pInst->NumLinks(eIncoming); iLink++)
                        LinkIndices << iLink;

                    CDeleteLinksCommand *pCmd = new CDeleteLinksCommand(this, pInst, eIncoming, LinkIndices);
                    mUndoStack.push(pCmd);
                }

                if (UnlinkOutgoing)
                {
                    QVector<u32> LinkIndices;
                    for (u32 iLink = 0; iLink < pInst->NumLinks(eOutgoing); iLink++)
                        LinkIndices << iLink;

                    CDeleteLinksCommand *pCmd = new CDeleteLinksCommand(this, pInst, eOutgoing, LinkIndices);
                    mUndoStack.push(pCmd);
                }
            }

            mUndoStack.endMacro();
        }
    }
}

void CWorldEditor::OnUndoStackIndexChanged()
{
    // Check the commands that have been executed on the undo stack and find out whether any of them affect the clean state.
    // This is to prevent commands like select/deselect from altering the clean state.
    int CurrentIndex = mUndoStack.index();
    int CleanIndex = mUndoStack.cleanIndex();

    if (CurrentIndex == CleanIndex)
        setWindowModified(false);

    else
    {
        bool IsClean = true;
        int LowIndex = (CurrentIndex > CleanIndex ? CleanIndex + 1 : CurrentIndex);
        int HighIndex = (CurrentIndex > CleanIndex ? CurrentIndex - 1 : CleanIndex);

        for (int iIdx = LowIndex; iIdx <= HighIndex; iIdx++)
        {
            const QUndoCommand *pkQCmd = mUndoStack.command(iIdx);

            if (const IUndoCommand *pkCmd = dynamic_cast<const IUndoCommand*>(pkQCmd))
            {
                if (pkCmd->AffectsCleanState())
                    IsClean = false;
            }

            else if (pkQCmd->childCount() > 0)
            {
                for (int iChild = 0; iChild < pkQCmd->childCount(); iChild++)
                {
                    const IUndoCommand *pkCmd = static_cast<const IUndoCommand*>(pkQCmd->child(iChild));

                    if (pkCmd->AffectsCleanState())
                    {
                        IsClean = false;
                        break;
                    }
                }
            }

            if (!IsClean) break;
        }

        setWindowModified(!IsClean);
    }
}

void CWorldEditor::OnPickModeEnter(QCursor Cursor)
{
    ui->MainViewport->SetCursorState(Cursor);
}

void CWorldEditor::OnPickModeExit()
{
    UpdateCursor();
}

void CWorldEditor::RefreshViewport()
{
    if (!mGizmo.IsTransforming())
        mGizmo.ResetSelectedAxes();

    // Process input
    ui->MainViewport->ProcessInput();

    // Update new link line
    UpdateNewLinkLine();

    // Render
    ui->MainViewport->Render();
}

void CWorldEditor::UpdateCameraOrbit()
{
    CCamera *pCamera = &ui->MainViewport->Camera();

    if (!mpSelection->IsEmpty())
        pCamera->SetOrbit(mpSelection->Bounds());
    else if (mpArea)
        pCamera->SetOrbit(mpArea->AABox(), 1.5f);
}

void CWorldEditor::OnCameraSpeedChange(double speed)
{
    static const double skDefaultSpeed = 1.0;
    ui->MainViewport->Camera().SetMoveSpeed(skDefaultSpeed * speed);

    ui->CamSpeedSpinBox->blockSignals(true);
    ui->CamSpeedSpinBox->setValue(speed);
    ui->CamSpeedSpinBox->blockSignals(false);
}

void CWorldEditor::OnTransformSpinBoxModified(CVector3f value)
{
    if (mpSelection->IsEmpty()) return;

    switch (mGizmo.Mode())
    {
        // Use absolute position/rotation, but relative scale. (This way spinbox doesn't show preview multiplier)
        case CGizmo::eTranslate:
        {
            CVector3f delta = value - mpSelection->Front()->AbsolutePosition();
            mUndoStack.push(new CTranslateNodeCommand(this, mpSelection->SelectedNodeList(), delta, mTranslateSpace));
            break;
        }

        case CGizmo::eRotate:
        {
            CQuaternion delta = CQuaternion::FromEuler(value) * mpSelection->Front()->AbsoluteRotation().Inverse();
            mUndoStack.push(new CRotateNodeCommand(this, mpSelection->SelectedNodeList(), CVector3f::skZero, delta, mRotateSpace));
            break;
        }

        case CGizmo::eScale:
        {
            CVector3f delta = value / mpSelection->Front()->AbsoluteScale();
            mUndoStack.push(new CScaleNodeCommand(this, mpSelection->SelectedNodeList(), CVector3f::skZero, delta));
            break;
        }
    }

    UpdateGizmoUI();
}

void CWorldEditor::OnTransformSpinBoxEdited(CVector3f)
{
    // bit of a hack - the vector editor emits a second "editing done" signal when it loses focus
    ui->TransformSpinBox->blockSignals(true);
    ui->MainViewport->setFocus();
    ui->TransformSpinBox->blockSignals(false);
    if (mpSelection->IsEmpty()) return;

    if (mGizmo.Mode() == CGizmo::eTranslate)   mUndoStack.push(CTranslateNodeCommand::End());
    else if (mGizmo.Mode() == CGizmo::eRotate) mUndoStack.push(CRotateNodeCommand::End());
    else if (mGizmo.Mode() == CGizmo::eScale)  mUndoStack.push(CScaleNodeCommand::End());

    UpdateGizmoUI();
}

void CWorldEditor::OnClosePoiEditDialog()
{
    delete mpPoiDialog;
    mpPoiDialog = nullptr;
    ui->MainViewport->SetRenderMergedWorld(true);
}

// These functions are from "Go to slot" in the designer
void CWorldEditor::on_ActionDrawWorld_triggered()
{
    ui->MainViewport->SetShowWorld(ui->ActionDrawWorld->isChecked());
}

void CWorldEditor::on_ActionDrawCollision_triggered()
{
    ui->MainViewport->SetShowFlag(eShowWorldCollision, ui->ActionDrawCollision->isChecked());
}

void CWorldEditor::on_ActionDrawObjects_triggered()
{
    ui->MainViewport->SetShowFlag(eShowObjectGeometry, ui->ActionDrawObjects->isChecked());
}

void CWorldEditor::on_ActionDrawLights_triggered()
{
    ui->MainViewport->SetShowFlag(eShowLights, ui->ActionDrawLights->isChecked());
}

void CWorldEditor::on_ActionDrawSky_triggered()
{
    ui->MainViewport->SetShowFlag(eShowSky, ui->ActionDrawSky->isChecked());
}

void CWorldEditor::on_ActionNoLighting_triggered()
{
    CGraphics::sLightMode = CGraphics::eNoLighting;
    ui->ActionNoLighting->setChecked(true);
    ui->ActionBasicLighting->setChecked(false);
    ui->ActionWorldLighting->setChecked(false);
}

void CWorldEditor::on_ActionBasicLighting_triggered()
{
    CGraphics::sLightMode = CGraphics::eBasicLighting;
    ui->ActionNoLighting->setChecked(false);
    ui->ActionBasicLighting->setChecked(true);
    ui->ActionWorldLighting->setChecked(false);
}

void CWorldEditor::on_ActionWorldLighting_triggered()
{
    CGraphics::sLightMode = CGraphics::eWorldLighting;
    ui->ActionNoLighting->setChecked(false);
    ui->ActionBasicLighting->setChecked(false);
    ui->ActionWorldLighting->setChecked(true);
}

void CWorldEditor::on_ActionNoBloom_triggered()
{
    ui->MainViewport->Renderer()->SetBloom(CRenderer::eNoBloom);
    ui->ActionNoBloom->setChecked(true);
    ui->ActionBloomMaps->setChecked(false);
    ui->ActionFakeBloom->setChecked(false);
    ui->ActionBloom->setChecked(false);
}

void CWorldEditor::on_ActionBloomMaps_triggered()
{
    ui->MainViewport->Renderer()->SetBloom(CRenderer::eBloomMaps);
    ui->ActionNoBloom->setChecked(false);
    ui->ActionBloomMaps->setChecked(true);
    ui->ActionFakeBloom->setChecked(false);
    ui->ActionBloom->setChecked(false);
}

void CWorldEditor::on_ActionFakeBloom_triggered()
{
    ui->MainViewport->Renderer()->SetBloom(CRenderer::eFakeBloom);
    ui->ActionNoBloom->setChecked(false);
    ui->ActionBloomMaps->setChecked(false);
    ui->ActionFakeBloom->setChecked(true);
    ui->ActionBloom->setChecked(false);
}

void CWorldEditor::on_ActionBloom_triggered()
{
    ui->MainViewport->Renderer()->SetBloom(CRenderer::eBloom);
    ui->ActionNoBloom->setChecked(false);
    ui->ActionBloomMaps->setChecked(false);
    ui->ActionFakeBloom->setChecked(false);
    ui->ActionBloom->setChecked(true);
}

void CWorldEditor::on_ActionDisableBackfaceCull_triggered()
{
    ui->MainViewport->Renderer()->ToggleBackfaceCull(!ui->ActionDisableBackfaceCull->isChecked());
}

void CWorldEditor::on_ActionDisableAlpha_triggered()
{
    ui->MainViewport->Renderer()->ToggleAlphaDisabled(ui->ActionDisableAlpha->isChecked());
}

void CWorldEditor::on_ActionEditLayers_triggered()
{
    // Launch layer editor
    CLayerEditor Editor(this);
    Editor.SetArea(mpArea);
    Editor.exec();
}

void CWorldEditor::on_ActionIncrementGizmo_triggered()
{
    mGizmo.IncrementSize();
}

void CWorldEditor::on_ActionDecrementGizmo_triggered()
{
    mGizmo.DecrementSize();
}

void CWorldEditor::on_ActionDrawObjectCollision_triggered()
{
    ui->MainViewport->SetShowFlag(eShowObjectCollision, ui->ActionDrawObjectCollision->isChecked());
}

void CWorldEditor::on_ActionGameMode_triggered()
{
    ui->MainViewport->SetGameMode(ui->ActionGameMode->isChecked());
}

void CWorldEditor::on_ActionSelectAll_triggered()
{
    FNodeFlags NodeFlags = CScene::NodeFlagsForShowFlags(ui->MainViewport->ShowFlags());
    NodeFlags &= ~(eModelNode | eStaticNode | eCollisionNode);
    SelectAll(NodeFlags);
}

void CWorldEditor::on_ActionInvertSelection_triggered()
{
    FNodeFlags NodeFlags = CScene::NodeFlagsForShowFlags(ui->MainViewport->ShowFlags());
    NodeFlags &= ~(eModelNode | eStaticNode | eCollisionNode);
    InvertSelection(NodeFlags);
}

void CWorldEditor::on_ActionEditPoiToWorldMap_triggered()
{
    if (!mpPoiDialog)
    {
        mpPoiDialog = new CPoiMapEditDialog(this, this);
        mpPoiDialog->show();
        ui->MainViewport->SetRenderMergedWorld(false);
        connect(mpPoiDialog, SIGNAL(Closed()), this, SLOT(OnClosePoiEditDialog()));
    }
    else
    {
        mpPoiDialog->show();
    }
}
