#include "Editor/WorldEditor/CWorldEditor.h"
#include "ui_CWorldEditor.h"

#include "Editor/CAboutDialog.h"
#include "Editor/CBasicViewport.h"
#include "Editor/CEditorApplication.h"
#include "Editor/CExportGameDialog.h"
#include "Editor/CGeneratePropertyNamesDialog.h"
#include "Editor/CLogDialog.h"
#include "Editor/CNodeCopyMimeData.h"
#include "Editor/CProjectSettingsDialog.h"
#include "Editor/CQuickplayPropertyEditor.h"
#include "Editor/CTweakEditor.h"
#include "Editor/UICommon.h"
#include "Editor/PropertyEdit/CPropertyView.h"
#include "Editor/ResourceBrowser/CResourceBrowser.h"
#include "Editor/Undo/UndoCommands.h"
#include "Editor/Widgets/WDraggableSpinBox.h"
#include "Editor/Widgets/WVectorEditor.h"
#include "Editor/WorldEditor/CCollisionRenderSettingsDialog.h"
#include "Editor/WorldEditor/CConfirmUnlinkDialog.h"
#include "Editor/WorldEditor/CLayerEditor.h"
#include "Editor/WorldEditor/CLinkDialog.h"
#include "Editor/WorldEditor/CPoiMapSidebar.h"
#include "Editor/WorldEditor/CScriptEditSidebar.h"
#include "Editor/WorldEditor/CTemplateMimeData.h"
#include "Editor/WorldEditor/CWorldInfoSidebar.h"
#include "Editor/WorldEditor/WCreateTab.h"
#include "Editor/WorldEditor/WModifyTab.h"

#include <Common/FileUtil.h>
#include <Common/Log.h>
#include <Core/SRayIntersection.h>
#include <Core/GameProject/CGameProject.h>
#include <Core/Resource/CWorld.h>
#include <Core/Resource/Area/CGameArea.h>
#include <Core/Resource/Script/NGameList.h>

#include <QButtonGroup>
#include <QClipboard>
#include <QComboBox>
#include <QFontMetrics>
#include <QMenu>
#include <QMessageBox>
#include <QSettings>
#include <QToolButton>
#include <QVBoxLayout>

#include <algorithm>

CWorldEditor::CWorldEditor(QWidget *parent)
    : INodeEditor(parent)
    , ui(std::make_unique<Ui::CWorldEditor>())
    , mpLinkDialog(new CLinkDialog(this, this))
    , mpGeneratePropertyNamesDialog(new CGeneratePropertyNamesDialog(this))
    , mpTweakEditor(new CTweakEditor(this))
{
    NLog::Debug("Creating World Editor");
    ui->setupUi(this);
    UpdateWindowTitle();

    mpSelection->SetAllowedNodeTypes(ENodeType::Script | ENodeType::Light);

    // Add resource browser to the layout
    auto* pLayout = new QVBoxLayout();
    pLayout->setContentsMargins(0,0,0,0);

    auto* pResourceBrowser = gpEdApp->ResourceBrowser();
    pResourceBrowser->setParent(this);
    pLayout->addWidget(pResourceBrowser);

    ui->ResourceBrowserContainer->setLayout(pLayout);

    // Initialize splitter
    ui->splitter->setSizes({
        static_cast<int>(width() * 0.25f),
        static_cast<int>(width() * 0.53f),
        static_cast<int>(width() * 0.22f),
    });

    // Initialize UI stuff
    ResetCamera();
    ui->MainViewport->SetScene(this, &mScene);
    ui->MainViewport->setAcceptDrops(true);
    ui->TransformSpinBox->SetOrientation(Qt::Horizontal);
    ui->TransformSpinBox->layout()->setContentsMargins(0,0,0,0);
    ui->CamSpeedSpinBox->SetDefaultValue(1.0);

    mpTransformCombo->setMinimumWidth(75);
    ui->MainToolBar->addActions(mGizmoActions);
    ui->MainToolBar->addWidget(mpTransformCombo);
    AddUndoActions(ui->menuEdit, ui->ActionCut);
    ui->menuEdit->insertSeparator(ui->ActionCut);

    // Initialize sidebar
    mpCurSidebar = nullptr;
    mpRightSidebarLayout = new QVBoxLayout();
    mpRightSidebarLayout->setContentsMargins(0, 0, 0, 0);
    ui->RightSidebarFrame->setLayout(mpRightSidebarLayout);

    mpWorldInfoSidebar = new CWorldInfoSidebar(this);
    mpScriptSidebar = new CScriptEditSidebar(this);
    mpPoiMapSidebar = new CPoiMapSidebar(this);

    // Initialize edit mode toolbar
    mpEditModeButtonGroup = new QButtonGroup(this);
    connect(mpEditModeButtonGroup, &QButtonGroup::idClicked, this, qOverload<int>(&CWorldEditor::ChangeEditMode));

    AddEditModeButton(QIcon(QStringLiteral(":/icons/World.svg")), tr("Edit World Info"), eWEM_EditWorldInfo);
    AddEditModeButton(QIcon(QStringLiteral(":/icons/Modify.svg")), tr("Edit Script"), eWEM_EditScript);
    mpPoiMapAction = AddEditModeButton(QIcon(QStringLiteral(":/icons/PoiSymbol_24px.svg")), tr("Edit POI Mappings"), eWEM_EditPOIMappings);
    mpPoiMapAction->setVisible(false);

    ChangeEditMode(eWEM_EditWorldInfo);

    // Initialize actions
    addAction(ui->ActionIncrementGizmo);
    addAction(ui->ActionDecrementGizmo);

    AddUndoActions(ui->MainToolBar, ui->ActionLink);
    ui->MainToolBar->insertSeparator(ui->ActionLink);

    ui->ActionCut->setAutoRepeat(false);
    ui->ActionCut->setShortcut(QKeySequence::Cut);
    ui->ActionCopy->setAutoRepeat(false);
    ui->ActionCopy->setShortcut(QKeySequence::Copy);
    ui->ActionPaste->setAutoRepeat(false);
    ui->ActionPaste->setShortcut(QKeySequence::Paste);
    ui->ActionDelete->setAutoRepeat(false);
    ui->ActionDelete->setShortcut(QKeySequence::Delete);

    mpCollisionDialog = new CCollisionRenderSettingsDialog(this, this);

    // Quickplay buttons
    auto* pQuickplayButton = new QToolButton(this);
    pQuickplayButton->setIcon(QIcon(QStringLiteral(":/icons/Play_32px.svg")));
    pQuickplayButton->setPopupMode(QToolButton::MenuButtonPopup);
    pQuickplayButton->setMenu(new CQuickplayPropertyEditor(mQuickplayParms, this));
    pQuickplayButton->setToolTip(tr("Quickplay"));

    ui->MainToolBar->addSeparator();
    mpQuickplayAction = ui->MainToolBar->addWidget(pQuickplayButton);
    mpQuickplayAction->setVisible(false);
    mpQuickplayAction->setEnabled(false);

    connect(pQuickplayButton, &QToolButton::pressed, this, &CWorldEditor::LaunchQuickplay);

    // "Open Recent" menu
    mpOpenRecentMenu = new QMenu(this);
    ui->ActionOpenRecent->setMenu(mpOpenRecentMenu);

    for (int iAct = 0; iAct < mskMaxRecentProjects; iAct++)
    {
        auto* pAction = new QAction(this);
        pAction->setVisible(false);
        pAction->setData(iAct);
        connect(pAction, &QAction::triggered, this, &CWorldEditor::OpenRecentProject);

        mpOpenRecentMenu->addAction(pAction);
        mRecentProjectsActions[iAct] = pAction;
    }
    UpdateOpenRecentActions();

    // Connect signals and slots
    connect(gpEdApp, &CEditorApplication::ActiveProjectChanged, this, &CWorldEditor::OnActiveProjectChanged);
    connect(gpEdApp->clipboard(), &QClipboard::dataChanged, this, &CWorldEditor::OnClipboardDataModified);

    connect(ui->MainViewport, &CSceneViewport::ViewportClick, this, &CWorldEditor::OnViewportClick);
    connect(ui->MainViewport, &CSceneViewport::InputProcessed, this, &CWorldEditor::OnViewportInputProcessed);
    connect(ui->MainViewport, &CSceneViewport::InputProcessed, this, &CWorldEditor::UpdateGizmoUI);
    connect(ui->MainViewport, &CSceneViewport::InputProcessed, this, &CWorldEditor::UpdateStatusBar);
    connect(ui->MainViewport, &CSceneViewport::InputProcessed, this, &CWorldEditor::UpdateCursor);
    connect(ui->MainViewport, &CSceneViewport::GizmoMoved, this, &CWorldEditor::OnGizmoMoved);
    connect(ui->MainViewport, &CSceneViewport::CameraOrbit, this, &CWorldEditor::UpdateCameraOrbit);
    connect(ui->MainViewport, &CSceneViewport::PlayFromHere, this, &CWorldEditor::OnPlayFromHere);
    connect(this, &CWorldEditor::SelectionModified, this, &CWorldEditor::OnSelectionModified);
    connect(this, &CWorldEditor::SelectionTransformed, this, &CWorldEditor::UpdateCameraOrbit);
    connect(this, &CWorldEditor::PickModeEntered, this, &CWorldEditor::OnPickModeEnter);
    connect(this, &CWorldEditor::PickModeExited, this, &CWorldEditor::OnPickModeExit);
    connect(this, &INodeEditor::NodeAboutToBeDeleted, this, &CWorldEditor::OnNodeAboutToBeDeleted);
    connect(ui->TransformSpinBox, &WVectorEditor::ValueChanged, this, &CWorldEditor::OnTransformSpinBoxModified);
    connect(ui->TransformSpinBox, &WVectorEditor::EditingDone, this, &CWorldEditor::OnTransformSpinBoxEdited);
    connect(ui->CamSpeedSpinBox, &WDraggableSpinBox::valueChanged, this, &CWorldEditor::OnCameraSpeedChange);
    connect(&UndoStack(), &QUndoStack::indexChanged, this, &CWorldEditor::OnUndoStackIndexChanged);

    connect(ui->ActionOpenProject, &QAction::triggered, this, &CWorldEditor::OpenProject);
    connect(ui->ActionSave, &QAction::triggered , this, &CWorldEditor::Save);
    connect(ui->ActionSaveAndRepack, &QAction::triggered, this, &CWorldEditor::SaveAndRepack);
    connect(ui->ActionExportGame, &QAction::triggered, this, &CWorldEditor::ExportGame);
    connect(ui->ActionProjectSettings, &QAction::triggered, this, &CWorldEditor::OpenProjectSettings);
    connect(ui->ActionCloseProject, &QAction::triggered, this, &CWorldEditor::CloseProject);
    connect(ui->ActionExit, &QAction::triggered, this, &CWorldEditor::close);

    connect(ui->ActionCut, &QAction::triggered, this, &CWorldEditor::Cut);
    connect(ui->ActionCopy, &QAction::triggered, this, &CWorldEditor::Copy);
    connect(ui->ActionPaste, &QAction::triggered, this, &CWorldEditor::Paste);
    connect(ui->ActionDelete, &QAction::triggered, this, &CWorldEditor::DeleteSelection);
    connect(ui->ActionSelectAll, &QAction::triggered, this, &CWorldEditor::SelectAllTriggered);
    connect(ui->ActionInvertSelection, &QAction::triggered, this, &CWorldEditor::InvertSelectionTriggered);
    connect(ui->ActionLink, &QAction::toggled, this, &CWorldEditor::OnLinkButtonToggled);
    connect(ui->ActionUnlink, &QAction::triggered, this, &CWorldEditor::OnUnlinkClicked);

    connect(ui->ActionEditTweaks, &QAction::triggered, mpTweakEditor, &CWorldEditor::show);
    connect(ui->ActionEditLayers, &QAction::triggered, this, &CWorldEditor::EditLayers);
    if (gTemplatesWritable)
        connect(ui->ActionGeneratePropertyNames, &QAction::triggered, mpGeneratePropertyNamesDialog, &CWorldEditor::show);
    else
        ui->ActionGeneratePropertyNames->setEnabled(false);

    connect(ui->ActionDrawWorld, &QAction::triggered, this, &CWorldEditor::ToggleDrawWorld);
    connect(ui->ActionDrawObjects, &QAction::triggered, this, &CWorldEditor::ToggleDrawObjects);
    connect(ui->ActionDrawCollision, &QAction::triggered, this, &CWorldEditor::ToggleDrawCollision);
    connect(ui->ActionDrawObjectCollision, &QAction::triggered, this, &CWorldEditor::ToggleDrawObjectCollision);
    connect(ui->ActionDrawLights, &QAction::triggered, this, &CWorldEditor::ToggleDrawLights);
    connect(ui->ActionDrawSky, &QAction::triggered, this, &CWorldEditor::ToggleDrawSky);
    connect(ui->ActionGameMode, &QAction::triggered, this, &CWorldEditor::ToggleGameMode);
    connect(ui->ActionDisableAlpha, &QAction::triggered, this, &CWorldEditor::ToggleDisableAlpha);
    connect(ui->ActionShowLog, &QAction::triggered, this, &CWorldEditor::OnShowLogClicked);
    connect(ui->ActionNoLighting, &QAction::triggered, this, &CWorldEditor::SetNoLighting);
    connect(ui->ActionBasicLighting, &QAction::triggered, this, &CWorldEditor::SetBasicLighting);
    connect(ui->ActionWorldLighting, &QAction::triggered, this, &CWorldEditor::SetWorldLighting);
    connect(ui->ActionNoBloom, &QAction::triggered, this, &CWorldEditor::SetNoBloom);
    connect(ui->ActionBloomMaps, &QAction::triggered, this, &CWorldEditor::SetBloomMaps);
    connect(ui->ActionFakeBloom, &QAction::triggered, this, &CWorldEditor::SetFakeBloom);
    connect(ui->ActionBloom, &QAction::triggered, this, &CWorldEditor::SetBloom);
    connect(ui->ActionIncrementGizmo, &QAction::triggered, this, &CWorldEditor::IncrementGizmo);
    connect(ui->ActionDecrementGizmo, &QAction::triggered, this, &CWorldEditor::DecrementGizmo);
    connect(ui->ActionCollisionRenderSettings, &QAction::triggered, mpCollisionDialog, &CWorldEditor::show);

    connect(ui->ActionAbout, &QAction::triggered, this, &CWorldEditor::About);
}

CWorldEditor::~CWorldEditor()
{
    // During destruction we need to ensure that no scene nodes are selected, as
    // CNodeSelection will attempt to unselect all nodes on destruction. This is
    // usually fine, however, in the event of the whole application shutting down,
    // this must come first, since the ClearScene() call below will remove all tracked
    // CSceneNode instances. If we don't do this, there's potential of a crash due to
    // CNodeSelection attempting to access nodes that no longer exist.
    //
    // We also block the Modified signal that would be emitted on clearing, since the
    // application is shutting down, so any UI updates aren't meaningful here.
    {
        [[maybe_unused]] const QSignalBlocker blocker{mpSelection.get()};
        mpSelection->Clear();
    }

    mScene.ClearScene();
    mpArea = nullptr;
    mpWorld = nullptr;
    if (gpResourceStore)
        gpResourceStore->DestroyUnreferencedResources(); // this should destroy the area!

    delete mpScriptSidebar; // For some reason WCreateTab filters an event during the viewport's destructor
}

bool CWorldEditor::CloseWorld()
{
    if (CheckUnsavedChanges())
    {
        ExitPickMode();
        ClearSelection();
        ui->MainViewport->ResetHover();
        mScene.ClearScene();

        UndoStack().clear();
        mpCollisionDialog->close();
        mpLinkDialog->close();
        mpQuickplayAction->setEnabled(false);

        mpArea = nullptr;
        mpWorld = nullptr;
        if (gpResourceStore)
            gpResourceStore->DestroyUnreferencedResources(); // this should destroy the area!
        UpdateWindowTitle();

        ui->ActionSave->setEnabled(false);
        ui->ActionSaveAndRepack->setEnabled(false);
        ui->ActionEditLayers->setEnabled(false);
        emit MapChanged(mpWorld, mpArea);
        return true;
    }

    return false;
}

bool CWorldEditor::SetArea(CWorld *pWorld, int AreaIndex)
{
    if (!CloseWorld())
        return false;

    ExitPickMode();
    ui->MainViewport->ResetHover();
    ClearSelection();
    UndoStack().clear();

    // Load new area
    mpWorld = pWorld;
    const CAssetID& AreaID = mpWorld->AreaResourceID(AreaIndex);
    CResourceEntry *pAreaEntry = mpWorld->Entry()->ResourceStore()->FindEntry(AreaID);
    ASSERT(pAreaEntry);

    mpArea = pAreaEntry->Load();
    ASSERT(mpArea);
    mpWorld->SetAreaLayerInfo(mpArea);
    mScene.SetActiveArea(mpWorld, mpArea);

    // Snap camera to new area
    CCamera *pCamera = &ui->MainViewport->Camera();

    if (pCamera->MoveMode() == ECameraMoveMode::Free)
    {
        const CTransform4f& AreaTransform = mpArea->Transform();
        const CVector3f AreaPosition(AreaTransform[0][3], AreaTransform[1][3], AreaTransform[2][3]);
        pCamera->Snap(AreaPosition);
    }

    UpdateCameraOrbit();

    // Update UI stuff
    UpdateWindowTitle();

    CGameTemplate *pGame = NGameList::GetGameTemplate(mpArea->Game());
    mpLinkDialog->SetGame(pGame);

    const auto AreaName = mpWorld->AreaInGameName(AreaIndex);
    if (CurrentGame() < EGame::DKCReturns)
        NLog::Debug("Loaded area: {} ({})", mpArea->Entry()->Name(), AreaName);
    else
        NLog::Debug("Loaded level: World {} / Area {} ({})", mpWorld->Entry()->Name(), mpArea->Entry()->Name(), AreaName);

    // Update paste action
    OnClipboardDataModified();

    // Update toolbar actions
    ui->ActionSave->setEnabled(true);
    ui->ActionSaveAndRepack->setEnabled(true);
    ui->ActionEditLayers->setEnabled(true);
    mpQuickplayAction->setEnabled(true);

    // Emit signals
    emit MapChanged(mpWorld, mpArea);
    emit LayersModified();

    return true;
}

void CWorldEditor::ResetCamera()
{
    ui->MainViewport->Camera().Snap(CVector3f(0.f, 5.f, 1.f));
}

bool CWorldEditor::HasAnyScriptNodesSelected() const
{
    return std::ranges::any_of(mpSelection->Nodes(), [](const auto* node) {
        return node->NodeType() == ENodeType::Script;
    });
}

bool CWorldEditor::IsQuickplayEnabled() const
{
    return mpQuickplayAction->isVisible() && mpQuickplayAction->isEnabled();
}

EGame CWorldEditor::CurrentGame() const
{
    return gpEdApp->CurrentGame();
}

CSceneViewport* CWorldEditor::Viewport() const
{
    return ui->MainViewport;
}

// ************ PUBLIC SLOTS ************
void CWorldEditor::EditorTick(float)
{
    // Update new link line
    UpdateNewLinkLine();
}

bool CWorldEditor::Save()
{
    if (!mpArea)
        return true;

    const bool SaveAreaSuccess = mpArea->Entry()->Save();
    const bool SaveEGMCSuccess = mpArea->PoiToWorldMap() ? mpArea->PoiToWorldMap()->Entry()->Save() : true;
    const bool SaveWorldSuccess = mpWorld->Entry()->Save();

    if (SaveAreaSuccess)
        mpArea->ClearExtraDependencies();

    if (SaveAreaSuccess || SaveEGMCSuccess || SaveWorldSuccess)
        gpEdApp->NotifyAssetsModified();

    if (SaveAreaSuccess && SaveEGMCSuccess && SaveWorldSuccess)
    {
        UndoStack().setClean();
        setWindowModified(false);
        return true;
    }
    else
    {
        UICommon::ErrorMsg(this, tr("Area failed to save!"));
        return false;
    }
}

void CWorldEditor::Cut()
{
    if (!mpSelection->IsEmpty())
    {
        Copy();
        UndoStack().push(new CDeleteSelectionCommand(this, tr("Cut")));
    }
}

void CWorldEditor::Copy()
{
    if (mpSelection->IsEmpty())
        return;

    qApp->clipboard()->setMimeData(new CNodeCopyMimeData(this));
}

void CWorldEditor::Paste()
{
    if (const auto* pkMimeData = qobject_cast<const CNodeCopyMimeData*>(qApp->clipboard()->mimeData()))
    {
        if (pkMimeData->Game() == CurrentGame())
        {
            CVector3f PastePoint;

            if (ui->MainViewport->underMouse() && !ui->MainViewport->IsMouseInputActive())
            {
                PastePoint = ui->MainViewport->HoverPoint();
            }
            else
            {
                const CRay Ray = ui->MainViewport->Camera().CastRay(CVector2f::Zero());
                const SRayIntersection Intersect = ui->MainViewport->SceneRayCast(Ray);

                if (Intersect.Hit)
                    PastePoint = Intersect.HitPoint;
                else
                    PastePoint = Ray.PointOnRay(10.f);
            }

            UndoStack().push(new CPasteNodesCommand(this, mpScriptSidebar->CreateTab()->SpawnLayer(), PastePoint));
        }
    }
}

void CWorldEditor::OpenProject()
{
    UICommon::OpenProject();
}

void CWorldEditor::OpenRecentProject()
{
    const auto* pSender = qobject_cast<QAction*>(sender());
    if (!pSender)
        return;

    QSettings Settings;
    const QStringList RecentProjectsList = Settings.value(QStringLiteral("WorldEditor/RecentProjectsList")).toStringList();

    const int ProjIndex = pSender->data().toInt();
    const QString& ProjPath = RecentProjectsList[ProjIndex];
    gpEdApp->OpenProject(ProjPath);
}

void CWorldEditor::ExportGame()
{
    const QString IsoPath = UICommon::OpenFileDialog(this, tr("Select ISO"), QStringLiteral("*.iso *.gcm *.tgc *.wbfs *.nfs"));
    if (IsoPath.isEmpty())
        return;

    const QString ExportDir = UICommon::OpenDirDialog(this, tr("Select output export directory"));
    if (ExportDir.isEmpty())
        return;

    CExportGameDialog ExportDialog(IsoPath, ExportDir, this);
    if (ExportDialog.HasValidDisc())
        ExportDialog.exec();

    if (ExportDialog.ExportSucceeded())
    {
        const int OpenChoice = QMessageBox::information(this, tr("Export complete"), tr("Export finished successfully! Open new project?"), QMessageBox::Yes, QMessageBox::No);

        if (OpenChoice == QMessageBox::Yes)
            gpEdApp->OpenProject(ExportDialog.ProjectPath());
    }
}

void CWorldEditor:: CloseProject()
{
    gpEdApp->CloseProject();
    SET_WINDOWTITLE_APPVARS(QStringLiteral("%APP_FULL_NAME%"));
}

void CWorldEditor::About()
{
    CAboutDialog Dialog(this);
    Dialog.exec();
}

void CWorldEditor::ChangeEditMode(int Mode)
{
    // This function is connected to the edit mode QButtonGroup.
    ChangeEditMode(EWorldEditorMode(Mode));
}

void CWorldEditor::ChangeEditMode(EWorldEditorMode Mode)
{
    mpEditModeButtonGroup->blockSignals(true);
    mpEditModeButtonGroup->button(Mode)->setChecked(true);
    mpEditModeButtonGroup->blockSignals(false);

    switch (Mode)
    {
    case eWEM_EditWorldInfo:
        SetSidebar(mpWorldInfoSidebar);
        break;

    case eWEM_EditScript:
        SetSidebar(mpScriptSidebar);
        break;

    case eWEM_EditPOIMappings:
        SetSidebar(mpPoiMapSidebar);
        break;

    default:
        ASSERT(false);
        break;
    }
}

void CWorldEditor::SetRenderingMergedWorld(bool RenderMerged)
{
    Viewport()->SetRenderMergedWorld(RenderMerged);
}

void CWorldEditor::OpenProjectSettings()
{
    auto* pDialog = gpEdApp->ProjectDialog();
    pDialog->show();
    pDialog->raise();
}

void CWorldEditor::OnActiveProjectChanged(CGameProject *pProj)
{
    ui->ActionProjectSettings->setEnabled(pProj != nullptr);
    ui->ActionCloseProject->setEnabled(pProj != nullptr);
    mpPoiMapAction->setVisible(pProj != nullptr && pProj->Game() >= EGame::EchoesDemo && pProj->Game() <= EGame::Corruption);
    mpQuickplayAction->setVisible(pProj != nullptr && NDolphinIntegration::IsQuickplaySupported(pProj));
    ResetCamera();
    UpdateWindowTitle();

    // Update tweak editor
    // We update this here to ensure we can update the menu item correctly without risking
    // that this function runs before the tweak editor has a chance to update its tweak list.
    mpTweakEditor->OnProjectChanged(pProj);
    ui->ActionEditTweaks->setEnabled(mpTweakEditor->HasTweaks());

    // Default bloom to Fake Bloom for Metroid Prime 3; disable for other games
    bool AllowBloom = (CurrentGame() == EGame::CorruptionProto || CurrentGame() == EGame::Corruption);
    AllowBloom ? SetFakeBloom() : SetNoBloom();
    ui->menuBloom->setEnabled(AllowBloom);

    // Update recent projects list
    UpdateOpenRecentActions();

    // Reset editor mode
    ChangeEditMode(eWEM_EditWorldInfo);
}

void CWorldEditor::OnPropertyModified(IProperty *pProp)
{
    bool ShouldUpdateSelection = false;

    for (auto* pNode : mpSelection->Nodes())
    {
        if (pNode && pNode->NodeType() == ENodeType::Script)
        {
            auto* pScript = static_cast<CScriptNode*>(pNode);
            pScript->PropertyModified(pProp);

            // If this is the name, update other parts of the UI to reflect the new value.
            if (pProp->Name() == "Name")
            {
                UpdateStatusBar();
                UpdateSelectionUI();
            }
            else if (pProp->Name() == "Position" ||
                     pProp->Name() == "Rotation" ||
                     pProp->Name() == "Scale")
            {
                mpSelection->UpdateBounds();
            }

            // Emit signal so other widgets can react to the property change
            emit PropertyModified(pProp, pScript->Instance());
        }

        // If this is a model/character, then we'll treat this as a modified selection. This is to make sure the selection bounds updates.
        if (pProp->Type() == EPropertyType::Asset)
        {
            const auto* pAsset = TPropCast<CAssetProperty>(pProp);
            const auto& rkFilter = pAsset->GetTypeFilter();

            if (rkFilter.Accepts(EResourceType::Model) || rkFilter.Accepts(EResourceType::AnimSet) || rkFilter.Accepts(EResourceType::Character))
                ShouldUpdateSelection = true;
        }
        else if (pProp->Type() == EPropertyType::AnimationSet)
        {
            ShouldUpdateSelection = true;
        }
    }

    if (ShouldUpdateSelection)
    {
        SelectionModified();
    }
}

void CWorldEditor::SetSelectionActive(bool Active)
{
    // Gather list of selected objects that actually have Active properties
    QList<CScriptObject*> Objects;

    for (auto* node : mpSelection->Nodes())
    {
        if (node->NodeType() == ENodeType::Script)
        {
            auto* pScript = static_cast<CScriptNode*>(node);
            auto* pInst = pScript->Instance();
            Objects.push_back(pInst);
        }
    }

    if (Objects.isEmpty())
        return;

    UndoStack().beginMacro(tr("Toggle Active"));

    while (!Objects.isEmpty())
    {
        QList<CScriptObject*> CommandObjects;
        CScriptTemplate* pTemplate = Objects[0]->Template();
        CBoolProperty* pActiveProperty = pTemplate->ActiveProperty();

        for (qsizetype ObjIdx = 0; ObjIdx < Objects.size(); ObjIdx++)
        {
            if (Objects[ObjIdx]->Template() == pTemplate)
            {
                CommandObjects.push_back(Objects[ObjIdx]);
                Objects.removeAt(ObjIdx);
                ObjIdx--;
            }
        }

        if (pActiveProperty)
        {
            auto* pModel = qobject_cast<CPropertyModel*>(mpScriptSidebar->ModifyTab()->PropertyView()->model());
            auto* pCommand = new CEditScriptPropertyCommand(pActiveProperty, CommandObjects, pModel);

            pCommand->SaveOldData();

            for (CScriptObject* pInstance : CommandObjects)
                pInstance->SetActive(Active);

            pCommand->SaveNewData();

            UndoStack().push(pCommand);
        }
    }

    UndoStack().endMacro();
}

void CWorldEditor::SetSelectionInstanceNames(const QString& rkNewName, bool IsDone)
{
    // todo: this only supports one node at a time because a macro prevents us from merging undo commands
    // this is fine right now because this function is only ever called with a selection of one node, but probably want to fix in the future
    /*if (mpSelection->Size() == 1 && mpSelection->Front()->NodeType() == eScriptNode)
    {
        auto* pNode = static_cast<CScriptNode*>(mpSelection->Front());
        auto* pInst = pNode->Instance();

        if (pName)
        {
            TString NewName = TO_TSTRING(rkNewName);
            IPropertyValue *pOld = pName->RawValue()->Clone();
            pInst->SetName(NewName);
            UndoStack().push(new CEditScriptPropertyCommand(pName, this, pOld, IsDone, "Edit Instance Name"));
        }
    }*/
}

void CWorldEditor::SetSelectionLayer(CScriptLayer *pLayer)
{
    QList<CScriptNode*> ScriptNodes;

    for (auto* node : mpSelection->Nodes())
    {
        if (node->NodeType() == ENodeType::Script)
            ScriptNodes.push_back(static_cast<CScriptNode*>(node));
    }

    if (!ScriptNodes.isEmpty())
        UndoStack().push(new CChangeLayerCommand(this, ScriptNodes, pLayer));
}

void CWorldEditor::DeleteSelection()
{
    if (!HasAnyScriptNodesSelected())
        return;

    UndoStack().push(new CDeleteSelectionCommand(this));
}

void CWorldEditor::UpdateOpenRecentActions()
{
    QSettings Settings;
    QStringList RecentProjectsList = Settings.value(QStringLiteral("WorldEditor/RecentProjectsList")).toStringList();

    // Bump the current project to the front
    if (const auto* pProj = gpEdApp->ActiveProject())
    {
        const QString ProjPath = TO_QSTRING(pProj->ProjectPath());
        RecentProjectsList.removeAll(ProjPath);
        RecentProjectsList.prepend(ProjPath);
    }

    // Remove projects that don't exist anymore
    for (const QString& rkProj : RecentProjectsList)
    {
        if (!FileUtil::Exists(TO_TSTRING(rkProj)) || rkProj.contains(QLatin1Char{'\\'}))
            RecentProjectsList.removeAll(rkProj);
    }

    Settings.setValue(QStringLiteral("WorldEditor/RecentProjectsList"), RecentProjectsList);

    // Set up the menu actions
    for (int iProj = 0; iProj < mskMaxRecentProjects; iProj++)
    {
        QAction* pAction = mRecentProjectsActions[iProj];

        if (iProj < RecentProjectsList.size())
        {
            const QString ActionText = tr("&%1 %2").arg(iProj).arg(RecentProjectsList[iProj]);
            pAction->setText(ActionText);
            pAction->setVisible(true);
        }
        else
        {
            pAction->setVisible(false);
        }
    }
}

void CWorldEditor::UpdateWindowTitle()
{
    QString WindowTitle = QStringLiteral("%APP_FULL_NAME%");

    if (const auto* pProj = gpEdApp->ActiveProject())
    {
        WindowTitle += QStringLiteral(" - ") + TO_QSTRING(pProj->Name());

        if (mpWorld)
        {
            WindowTitle += QStringLiteral(" - ") + TO_QSTRING(mpWorld->InGameName());

            if (mpArea && CurrentGame() < EGame::DKCReturns)
                WindowTitle += QStringLiteral(" - ") + TO_QSTRING(mpWorld->AreaInGameName(mpArea->WorldIndex()));
        }
    }

    WindowTitle += QStringLiteral("[*]");
    SET_WINDOWTITLE_APPVARS(WindowTitle);
}

void CWorldEditor::UpdateStatusBar()
{
    // Would be cool to do more frequent status bar updates with more info. Unfortunately, this causes lag.
    QString StatusText;

    if (!mGizmoHovering)
    {
        if (ui->MainViewport->underMouse())
        {
            const CSceneNode* pHoverNode = ui->MainViewport->HoverNode();

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
    if (!ui->TransformSpinBox->IsBeingEdited())
    {
        CVector3f SpinBoxValue = CVector3f::Zero();

        // If the gizmo is transforming, use the total transform amount
        // Otherwise, use the first selected node transform, or 0 if no selection
        if (mShowGizmo)
        {
            switch (mGizmo.Mode())
            {
            case CGizmo::EGizmoMode::Translate:
                if (mGizmoTransforming && mGizmo.HasTransformed())
                    SpinBoxValue = mGizmo.TotalTranslation();
                else if (!mpSelection->IsEmpty())
                    SpinBoxValue = mpSelection->Front()->AbsolutePosition();
                break;

            case CGizmo::EGizmoMode::Rotate:
                if (mGizmoTransforming && mGizmo.HasTransformed())
                    SpinBoxValue = mGizmo.TotalRotation();
                else if (!mpSelection->IsEmpty())
                    SpinBoxValue = mpSelection->Front()->AbsoluteRotation().ToEuler();
                break;

            case CGizmo::EGizmoMode::Scale:
                if (mGizmoTransforming && mGizmo.HasTransformed())
                    SpinBoxValue = mGizmo.TotalScale();
                else if (!mpSelection->IsEmpty())
                    SpinBoxValue = mpSelection->Front()->AbsoluteScale();
                break;

            default:
                break;
            }
        }
        else if (!mpSelection->IsEmpty())
        {
            SpinBoxValue = mpSelection->Front()->AbsolutePosition();
        }

        ui->TransformSpinBox->blockSignals(true);
        ui->TransformSpinBox->SetValue(SpinBoxValue);
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

    if (mpSelection->SingleNodeSelected())
        SelectionText = TO_QSTRING(mpSelection->Front()->Name());
    else if (mpSelection->MultipleNodesSelected())
        SelectionText = tr("%n objects selected", "", int(mpSelection->Size()));

    const QFontMetrics Metrics(ui->SelectionInfoLabel->font());
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
        const CSceneNode* pHoverNode = ui->MainViewport->HoverNode();

        if (ui->MainViewport->IsHoveringGizmo())
            ui->MainViewport->SetCursorState(Qt::SizeAllCursor);
        else if (pHoverNode && mpSelection->IsAllowedType(pHoverNode))
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
        const CVector3f& Start = mScene.NodeForInstance(mpLinkDialog->Sender())->CenterPoint();
        const CVector3f& End = mScene.NodeForInstance(mpLinkDialog->Receiver())->CenterPoint();
        ui->MainViewport->SetLinkLineEnabled(true);
        ui->MainViewport->SetLinkLine(Start, End);
    }
    else // Otherwise check whether there's just a sender or just a receiver
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
        {
            pSender = mpNewLinkSender;
        }
        else if (mpScriptSidebar->ModifyTab()->IsPicking() && mpScriptSidebar->ModifyTab()->EditNode()->NodeType() == ENodeType::Script)
        {
            pSender = static_cast<CScriptNode*>(mpScriptSidebar->ModifyTab()->EditNode())->Instance();
        }

        // No sender and no receiver = no line
        if (!pSender && !pReceiver)
        {
            ui->MainViewport->SetLinkLineEnabled(false);
        }
        // Yes sender and yes receiver = yes line
        else if (pSender && pReceiver)
        {
            ui->MainViewport->SetLinkLineEnabled(true);
            ui->MainViewport->SetLinkLine( mScene.NodeForInstance(pSender)->CenterPoint(), mScene.NodeForInstance(pReceiver)->CenterPoint() );
        }
        // Compensate for missing sender or missing receiver
        else
        {
            const bool IsPicking = (mIsMakingLink || mpLinkDialog->IsPicking() || mpScriptSidebar->ModifyTab()->IsPicking());

            if (ui->MainViewport->underMouse() && !ui->MainViewport->IsMouseInputActive() && IsPicking)
            {
                const CSceneNode* pHoverNode = ui->MainViewport->HoverNode();
                const CScriptObject* pInst = (pSender ? pSender : pReceiver);

                const CVector3f& Start = mScene.NodeForInstance(pInst)->CenterPoint();
                const CVector3f& End = (pHoverNode && pHoverNode->NodeType() == ENodeType::Script ? pHoverNode->CenterPoint() : ui->MainViewport->HoverPoint());
                ui->MainViewport->SetLinkLineEnabled(true);
                ui->MainViewport->SetLinkLine(Start, End);
            }
            else
            {
                ui->MainViewport->SetLinkLineEnabled(false);
            }
        }
    }
}

void CWorldEditor::LaunchQuickplay()
{
    const CVector3f& CameraPosition = Viewport()->Camera().Position();
    LaunchQuickplayFromLocation(CameraPosition, false);
}

void CWorldEditor::LaunchQuickplayFromLocation(const CVector3f& Location, bool ForceAsSpawnPosition)
{
    // This function should not be called if a level is not open in a project.
    ASSERT(gpEdApp->ActiveProject() != nullptr);
    ASSERT(mpWorld && mpArea);

    // Fill in parameters and start running
    SQuickplayParameters Parameters = mQuickplayParms;
    Parameters.BootWorldAssetID = mpWorld->ID().ToU32();
    Parameters.BootAreaAssetID = mpArea->ID().ToU32();
    Parameters.SpawnTransform = Viewport()->Camera().GetCameraTransform();
    Parameters.SpawnTransform.SetTranslation(Location);

    if (ForceAsSpawnPosition)
    {
        Parameters.Features.SetFlag(EQuickplayFeature::JumpToArea);
        Parameters.Features.SetFlag(EQuickplayFeature::SetSpawnPosition);
    }

    NDolphinIntegration::LaunchQuickplay(this, gpEdApp->ActiveProject(), Parameters);
}

// ************ PROTECTED ************
QAction* CWorldEditor::AddEditModeButton(const QIcon& Icon, const QString& ToolTip, EWorldEditorMode Mode)
{
    ASSERT(mpEditModeButtonGroup->button(Mode) == nullptr);

    auto* pButton = new QPushButton(Icon, {}, this);
    pButton->setCheckable(true);
    pButton->setToolTip(ToolTip);
    pButton->setIconSize(QSize(24, 24));

    auto* pAction = ui->EditModeToolBar->addWidget(pButton);
    mpEditModeButtonGroup->addButton(pButton, Mode);
    return pAction;
}

void CWorldEditor::SetSidebar(CWorldEditorSidebar *pSidebar)
{
    if (mpCurSidebar == pSidebar)
        return;

    if (mpCurSidebar)
    {
        mpCurSidebar->SidebarClose();
        mpRightSidebarLayout->removeWidget(mpCurSidebar);
        mpCurSidebar->setHidden(true);
    }

    mpCurSidebar = pSidebar;

    if (mpCurSidebar)
    {
        mpCurSidebar->SidebarOpen();
        mpRightSidebarLayout->addWidget(mpCurSidebar);
        mpCurSidebar->setHidden(false);
    }
}

void CWorldEditor::GizmoModeChanged(CGizmo::EGizmoMode mode)
{
    ui->TransformSpinBox->SetSingleStep((mode == CGizmo::EGizmoMode::Rotate ? 1.0 : 0.1));
    ui->TransformSpinBox->SetDefaultValue((mode == CGizmo::EGizmoMode::Scale ? 1.0 : 0.0));
}

// ************ PRIVATE SLOTS ************
void CWorldEditor::OnClipboardDataModified()
{
    const auto* pkClipboardMimeData = qApp->clipboard()->mimeData();
    const auto* pkMimeData = qobject_cast<const CNodeCopyMimeData*>(pkClipboardMimeData);
    const bool ValidMimeData = (pkMimeData && pkMimeData->Game() == CurrentGame());
    ui->ActionPaste->setEnabled(ValidMimeData);
}

void CWorldEditor::OnSelectionModified()
{
    ui->TransformSpinBox->setEnabled(!mpSelection->IsEmpty());

    const bool HasScriptNode = HasAnyScriptNodesSelected();
    ui->ActionCut->setEnabled(HasScriptNode);
    ui->ActionCopy->setEnabled(HasScriptNode);
    ui->ActionDelete->setEnabled(HasScriptNode);

    UpdateCameraOrbit();
}

void CWorldEditor::OnLinkButtonToggled(bool Enabled)
{
    if (Enabled)
    {
        EnterPickMode(ENodeType::Script, true, false, false);
        connect(this, &CWorldEditor::PickModeClick, this, &CWorldEditor::OnLinkClick);
        connect(this, &CWorldEditor::PickModeExited, this, &CWorldEditor::OnLinkEnd);
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
        mpNewLinkSender = static_cast<CScriptNode*>(rkIntersect.pNode)->Instance();
    }
    else
    {
        mpNewLinkReceiver = static_cast<CScriptNode*>(rkIntersect.pNode)->Instance();
        mpLinkDialog->NewLink(mpNewLinkSender, mpNewLinkReceiver);
        mpLinkDialog->show();
        ExitPickMode();
    }
}

void CWorldEditor::OnLinkEnd()
{
    disconnect(this, &CWorldEditor::PickModeClick, this, &CWorldEditor::OnLinkClick);
    disconnect(this, &CWorldEditor::PickModeExited, this, &CWorldEditor::OnLinkEnd);
    ui->ActionLink->setChecked(false);
    mIsMakingLink = false;
    mpNewLinkSender = nullptr;
    mpNewLinkReceiver = nullptr;
}

void CWorldEditor::OnUnlinkClicked()
{
    QList<CScriptNode*> SelectedScriptNodes;
    for (auto* node : mpSelection->Nodes())
    {
        if (node->NodeType() == ENodeType::Script)
            SelectedScriptNodes.push_back(static_cast<CScriptNode*>(node));
    }

    if (!SelectedScriptNodes.isEmpty())
    {
        CConfirmUnlinkDialog Dialog(this);
        Dialog.exec();

        if (Dialog.UserChoice() != CConfirmUnlinkDialog::EChoice::Cancel)
        {
            UndoStack().beginMacro(tr("Unlink"));
            const bool UnlinkIncoming = (Dialog.UserChoice() != CConfirmUnlinkDialog::EChoice::OutgoingOnly);
            const bool UnlinkOutgoing = (Dialog.UserChoice() != CConfirmUnlinkDialog::EChoice::IncomingOnly);

            for (CScriptNode *pNode : SelectedScriptNodes)
            {
                CScriptObject *pInst = pNode->Instance();

                if (UnlinkIncoming)
                {
                    QList<uint32_t> LinkIndices;
                    for (uint32_t iLink = 0; iLink < pInst->NumLinks(ELinkType::Incoming); iLink++)
                        LinkIndices.push_back(iLink);
;
                    UndoStack().push(new CDeleteLinksCommand(this, pInst, ELinkType::Incoming, LinkIndices));
                }

                if (UnlinkOutgoing)
                {
                    QList<uint32_t> LinkIndices;
                    for (uint32_t iLink = 0; iLink < pInst->NumLinks(ELinkType::Outgoing); iLink++)
                        LinkIndices.push_back(iLink);

                    UndoStack().push(new CDeleteLinksCommand(this, pInst, ELinkType::Outgoing, LinkIndices));
                }
            }

            UndoStack().endMacro();
        }
    }
}

void CWorldEditor::OnPickModeEnter(const QCursor& Cursor)
{
    ui->MainViewport->SetCursorState(Cursor);
}

void CWorldEditor::OnPickModeExit()
{
    UpdateCursor();
}

void CWorldEditor::UpdateCameraOrbit()
{
    CCamera* pCamera = &ui->MainViewport->Camera();

    if (!mpSelection->IsEmpty())
        pCamera->SetOrbit(mpSelection->Bounds());
    else if (mpArea)
        pCamera->SetOrbit(mpArea->AABox(), 1.2f);
    else
        pCamera->ResetOrbit();
}

void CWorldEditor::OnCameraSpeedChange(double Speed)
{
    ui->MainViewport->Camera().SetMoveSpeed(CCamera::default_move_speed * Speed);

    ui->CamSpeedSpinBox->blockSignals(true);
    ui->CamSpeedSpinBox->setValue(Speed);
    ui->CamSpeedSpinBox->blockSignals(false);
}

void CWorldEditor::OnTransformSpinBoxModified(const CVector3f& Value)
{
    if (mpSelection->IsEmpty())
        return;

    switch (mGizmo.Mode())
    {
    // Use absolute position/rotation, but relative scale. (This way spinbox doesn't show preview multiplier)
    case CGizmo::EGizmoMode::Translate:
    {
        const CVector3f Delta = Value - mpSelection->Front()->AbsolutePosition();
        UndoStack().push(new CTranslateNodeCommand(this, mpSelection->Nodes(), Delta, mTranslateSpace));
        break;
    }

    case CGizmo::EGizmoMode::Rotate:
    {
        const CQuaternion Delta = CQuaternion::FromEuler(Value) * mpSelection->Front()->AbsoluteRotation().Inverse();
        UndoStack().push(new CRotateNodeCommand(this, mpSelection->Nodes(), true, mGizmo.Position(), mGizmo.Rotation(), Delta, mRotateSpace));
        break;
    }

    case CGizmo::EGizmoMode::Scale:
    {
        const CVector3f Delta = Value / mpSelection->Front()->AbsoluteScale();
        UndoStack().push(new CScaleNodeCommand(this, mpSelection->Nodes(), true, mGizmo.Position(), Delta));
        break;
    }

    default:
        break;
    }

    UpdateGizmoUI();
}

void CWorldEditor::OnTransformSpinBoxEdited(const CVector3f&)
{
    if (mpSelection->IsEmpty())
        return;

    if (mGizmo.Mode() == CGizmo::EGizmoMode::Translate)   UndoStack().push(CTranslateNodeCommand::End());
    else if (mGizmo.Mode() == CGizmo::EGizmoMode::Rotate) UndoStack().push(CRotateNodeCommand::End());
    else if (mGizmo.Mode() == CGizmo::EGizmoMode::Scale)  UndoStack().push(CScaleNodeCommand::End());

    UpdateGizmoUI();
}

void CWorldEditor::OnNodeAboutToBeDeleted(const CSceneNode* node)
{
    if (ui->MainViewport->HoverNode() == node)
        ui->MainViewport->ResetHover();
}

void CWorldEditor::SelectAllTriggered()
{
    FNodeFlags NodeFlags = CScene::NodeFlagsForShowFlags(ui->MainViewport->ShowFlags());
    NodeFlags &= ~(ENodeType::Model | ENodeType::Static | ENodeType::Collision);
    SelectAll(NodeFlags);
}

void CWorldEditor::InvertSelectionTriggered()
{
    FNodeFlags NodeFlags = CScene::NodeFlagsForShowFlags(ui->MainViewport->ShowFlags());
    NodeFlags &= ~(ENodeType::Model | ENodeType::Static | ENodeType::Collision);
    InvertSelection(NodeFlags);
}

void CWorldEditor::ToggleDrawWorld()
{
    ui->MainViewport->SetShowWorld(ui->ActionDrawWorld->isChecked());
}

void CWorldEditor::ToggleDrawObjects()
{
    ui->MainViewport->SetShowFlag(EShowFlag::ObjectGeometry, ui->ActionDrawObjects->isChecked());
}

void CWorldEditor::ToggleDrawCollision()
{
    ui->MainViewport->SetShowFlag(EShowFlag::WorldCollision, ui->ActionDrawCollision->isChecked());
}

void CWorldEditor::ToggleDrawObjectCollision()
{
    ui->MainViewport->SetShowFlag(EShowFlag::ObjectCollision, ui->ActionDrawObjectCollision->isChecked());
}

void CWorldEditor::ToggleDrawLights()
{
    ui->MainViewport->SetShowFlag(EShowFlag::Lights, ui->ActionDrawLights->isChecked());
}

void CWorldEditor::ToggleDrawSky()
{
    ui->MainViewport->SetShowFlag(EShowFlag::Sky, ui->ActionDrawSky->isChecked());
}

void CWorldEditor::ToggleGameMode()
{
    ui->MainViewport->SetGameMode(ui->ActionGameMode->isChecked());
}

void CWorldEditor::ToggleDisableAlpha()
{
    ui->MainViewport->Renderer()->ToggleAlphaDisabled(ui->ActionDisableAlpha->isChecked());
}

void CWorldEditor::SetNoLighting()
{
    CGraphics::sLightMode = CGraphics::ELightingMode::None;
    ui->ActionNoLighting->setChecked(true);
    ui->ActionBasicLighting->setChecked(false);
    ui->ActionWorldLighting->setChecked(false);
}

void CWorldEditor::SetBasicLighting()
{
    CGraphics::sLightMode = CGraphics::ELightingMode::Basic;
    ui->ActionNoLighting->setChecked(false);
    ui->ActionBasicLighting->setChecked(true);
    ui->ActionWorldLighting->setChecked(false);
}

void CWorldEditor::SetWorldLighting()
{
    CGraphics::sLightMode = CGraphics::ELightingMode::World;
    ui->ActionNoLighting->setChecked(false);
    ui->ActionBasicLighting->setChecked(false);
    ui->ActionWorldLighting->setChecked(true);
}

void CWorldEditor::SetNoBloom()
{
    ui->MainViewport->Renderer()->SetBloom(EBloomMode::NoBloom);
    ui->ActionNoBloom->setChecked(true);
    ui->ActionBloomMaps->setChecked(false);
    ui->ActionFakeBloom->setChecked(false);
    ui->ActionBloom->setChecked(false);
}

void CWorldEditor::SetBloomMaps()
{
    ui->MainViewport->Renderer()->SetBloom(EBloomMode::BloomMaps);
    ui->ActionNoBloom->setChecked(false);
    ui->ActionBloomMaps->setChecked(true);
    ui->ActionFakeBloom->setChecked(false);
    ui->ActionBloom->setChecked(false);
}

void CWorldEditor::SetFakeBloom()
{
    ui->MainViewport->Renderer()->SetBloom(EBloomMode::FakeBloom);
    ui->ActionNoBloom->setChecked(false);
    ui->ActionBloomMaps->setChecked(false);
    ui->ActionFakeBloom->setChecked(true);
    ui->ActionBloom->setChecked(false);
}

void CWorldEditor::SetBloom()
{
    ui->MainViewport->Renderer()->SetBloom(EBloomMode::Bloom);
    ui->ActionNoBloom->setChecked(false);
    ui->ActionBloomMaps->setChecked(false);
    ui->ActionFakeBloom->setChecked(false);
    ui->ActionBloom->setChecked(true);
}

void CWorldEditor::IncrementGizmo()
{
    mGizmo.IncrementSize();
}

void CWorldEditor::DecrementGizmo()
{
    mGizmo.DecrementSize();
}

void CWorldEditor::EditLayers()
{
    // Launch layer editor
    CLayerEditor Editor(this);
    Editor.SetArea(mpArea);
    Editor.exec();
}

void CWorldEditor::OnShowLogClicked(bool show)
{
    if (!mpLogDialog)
    {
        mpLogDialog = new CLogDialog(this);

        // If the dialog is ever hidden or closed from within the dialog itself
        // we need to adjust UI state accordingly.
        connect(mpLogDialog, &QDialog::finished, [this] {
            ui->ActionShowLog->setChecked(false);
        });
    }

    mpLogDialog->setVisible(show);
}

void CWorldEditor::OnPlayFromHere(const CSceneNode* node, const CVector3f& position)
{
    if (node)
        LaunchQuickplayFromLocation(position, true);
    else
        LaunchQuickplay();
}
