#include "CWorldEditor.h"
#include "ui_CWorldEditor.h"
#include "CConfirmUnlinkDialog.h"
#include "CLayerEditor.h"
#include "CTemplateMimeData.h"
#include "WCreateTab.h"
#include "WModifyTab.h"
#include "WInstancesTab.h"

#include "Editor/CAboutDialog.h"
#include "Editor/CBasicViewport.h"
#include "Editor/CExportGameDialog.h"
#include "Editor/CNodeCopyMimeData.h"
#include "Editor/CProjectSettingsDialog.h"
#include "Editor/CQuickplayPropertyEditor.h"
#include "Editor/CSelectionIterator.h"
#include "Editor/UICommon.h"
#include "Editor/PropertyEdit/CPropertyView.h"
#include "Editor/ResourceBrowser/CResourceBrowser.h"
#include "Editor/Widgets/WDraggableSpinBox.h"
#include "Editor/Widgets/WVectorEditor.h"
#include "Editor/Undo/UndoCommands.h"

#include <Common/Log.h>
#include <Core/GameProject/CGameProject.h>
#include <Core/Render/CDrawUtil.h>
#include <Core/Resource/Script/NGameList.h>
#include <Core/Scene/CSceneIterator.h>

#include <QButtonGroup>
#include <QClipboard>
#include <QComboBox>
#include <QFontMetrics>
#include <QMessageBox>
#include <QSettings>
#include <QToolButton>

CWorldEditor::CWorldEditor(QWidget *parent)
    : INodeEditor(parent)
    , ui(std::make_unique<Ui::CWorldEditor>())
    , mpLinkDialog(new CLinkDialog(this, this))
    , mpGeneratePropertyNamesDialog(new CGeneratePropertyNamesDialog(this))
    , mpTweakEditor(new CTweakEditor(this))
{
    debugf("Creating World Editor");
    ui->setupUi(this);
    UpdateWindowTitle();

    mpSelection->SetAllowedNodeTypes(ENodeType::Script | ENodeType::Light);

    // Add resource browser to the layout
    QVBoxLayout *pLayout = new QVBoxLayout();
    pLayout->setContentsMargins(0,0,0,0);

    CResourceBrowser *pResourceBrowser = gpEdApp->ResourceBrowser();
    pResourceBrowser->setParent(this);
    pLayout->addWidget( pResourceBrowser );

    ui->ResourceBrowserContainer->setLayout(pLayout);

    // Initialize splitter
    const QList<int> SplitterSizes{
        static_cast<int>(width() * 0.25),
        static_cast<int>(width() * 0.53),
        static_cast<int>(width() * 0.22),
    };
    ui->splitter->setSizes(SplitterSizes);

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
    connect(mpEditModeButtonGroup, SIGNAL(buttonClicked(int)), this, SLOT(ChangeEditMode(int)));

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
    QToolButton* pQuickplayButton = new QToolButton(this);
    pQuickplayButton->setIcon(QIcon(QStringLiteral(":/icons/Play_32px.svg")));
    pQuickplayButton->setPopupMode(QToolButton::MenuButtonPopup);
    pQuickplayButton->setMenu(new CQuickplayPropertyEditor(mQuickplayParms, this));
    pQuickplayButton->setToolTip(tr("Quickplay"));

    ui->MainToolBar->addSeparator();
    mpQuickplayAction = ui->MainToolBar->addWidget(pQuickplayButton);
    mpQuickplayAction->setVisible(false);
    mpQuickplayAction->setEnabled(false);

    connect(pQuickplayButton, SIGNAL(pressed()), this, SLOT(LaunchQuickplay()));

    // "Open Recent" menu
    mpOpenRecentMenu = new QMenu(this);
    ui->ActionOpenRecent->setMenu(mpOpenRecentMenu);

    for (uint32 iAct = 0; iAct < mskMaxRecentProjects; iAct++)
    {
        QAction *pAction = new QAction(this);
        pAction->setVisible(false);
        pAction->setData((int) iAct);
        connect(pAction, SIGNAL(triggered(bool)), this, SLOT(OpenRecentProject()));

        mpOpenRecentMenu->addAction(pAction);
        mRecentProjectsActions[iAct] = pAction;
    }
    UpdateOpenRecentActions();

    // Connect signals and slots
    connect(gpEdApp, SIGNAL(ActiveProjectChanged(CGameProject*)), this, SLOT(OnActiveProjectChanged(CGameProject*)));
    connect(gpEdApp->clipboard(), SIGNAL(dataChanged()), this, SLOT(OnClipboardDataModified()));

    connect(ui->MainViewport, SIGNAL(ViewportClick(SRayIntersection,QMouseEvent*)), this, SLOT(OnViewportClick(SRayIntersection,QMouseEvent*)));
    connect(ui->MainViewport, SIGNAL(InputProcessed(SRayIntersection,QMouseEvent*)), this, SLOT(OnViewportInputProcessed(SRayIntersection,QMouseEvent*)));
    connect(ui->MainViewport, SIGNAL(InputProcessed(SRayIntersection,QMouseEvent*)), this, SLOT(UpdateGizmoUI()) );
    connect(ui->MainViewport, SIGNAL(InputProcessed(SRayIntersection,QMouseEvent*)), this, SLOT(UpdateStatusBar()) );
    connect(ui->MainViewport, SIGNAL(InputProcessed(SRayIntersection,QMouseEvent*)), this, SLOT(UpdateCursor()) );
    connect(ui->MainViewport, SIGNAL(GizmoMoved()), this, SLOT(OnGizmoMoved()));
    connect(ui->MainViewport, SIGNAL(CameraOrbit()), this, SLOT(UpdateCameraOrbit()));
    connect(this, SIGNAL(SelectionModified()), this, SLOT(OnSelectionModified()));
    connect(this, SIGNAL(SelectionTransformed()), this, SLOT(UpdateCameraOrbit()));
    connect(this, SIGNAL(PickModeEntered(QCursor)), this, SLOT(OnPickModeEnter(QCursor)));
    connect(this, SIGNAL(PickModeExited()), this, SLOT(OnPickModeExit()));
    connect(ui->TransformSpinBox, SIGNAL(ValueChanged(CVector3f)), this, SLOT(OnTransformSpinBoxModified(CVector3f)));
    connect(ui->TransformSpinBox, SIGNAL(EditingDone(CVector3f)), this, SLOT(OnTransformSpinBoxEdited(CVector3f)));
    connect(ui->CamSpeedSpinBox, SIGNAL(valueChanged(double)), this, SLOT(OnCameraSpeedChange(double)));
    connect(&UndoStack(), SIGNAL(indexChanged(int)), this, SLOT(OnUndoStackIndexChanged()));

    connect(ui->ActionOpenProject, SIGNAL(triggered()), this, SLOT(OpenProject()));
    connect(ui->ActionSave, SIGNAL(triggered()) , this, SLOT(Save()));
    connect(ui->ActionSaveAndRepack, SIGNAL(triggered()), this, SLOT(SaveAndRepack()));
    connect(ui->ActionExportGame, SIGNAL(triggered()), this, SLOT(ExportGame()));
    connect(ui->ActionProjectSettings, SIGNAL(triggered()), this, SLOT(OpenProjectSettings()));
    connect(ui->ActionCloseProject, SIGNAL(triggered()), this, SLOT(CloseProject()));
    connect(ui->ActionExit, SIGNAL(triggered()), this, SLOT(close()));

    connect(ui->ActionCut, SIGNAL(triggered()), this, SLOT(Cut()));
    connect(ui->ActionCopy, SIGNAL(triggered()), this, SLOT(Copy()));
    connect(ui->ActionPaste, SIGNAL(triggered()), this, SLOT(Paste()));
    connect(ui->ActionDelete, SIGNAL(triggered()), this, SLOT(DeleteSelection()));
    connect(ui->ActionSelectAll, SIGNAL(triggered()), this, SLOT(SelectAllTriggered()));
    connect(ui->ActionInvertSelection, SIGNAL(triggered()), this, SLOT(InvertSelectionTriggered()));
    connect(ui->ActionLink, SIGNAL(toggled(bool)), this, SLOT(OnLinkButtonToggled(bool)));
    connect(ui->ActionUnlink, SIGNAL(triggered()), this, SLOT(OnUnlinkClicked()));

    connect(ui->ActionEditTweaks, SIGNAL(triggered()), mpTweakEditor, SLOT(show()));
    connect(ui->ActionEditLayers, SIGNAL(triggered()), this, SLOT(EditLayers()));
    if (gTemplatesWritable)
        connect(ui->ActionGeneratePropertyNames, SIGNAL(triggered()), mpGeneratePropertyNamesDialog, SLOT(show()));
    else
        ui->ActionGeneratePropertyNames->setEnabled(false);

    connect(ui->ActionDrawWorld, SIGNAL(triggered()), this, SLOT(ToggleDrawWorld()));
    connect(ui->ActionDrawObjects, SIGNAL(triggered()), this, SLOT(ToggleDrawObjects()));
    connect(ui->ActionDrawCollision, SIGNAL(triggered()), this, SLOT(ToggleDrawCollision()));
    connect(ui->ActionDrawObjectCollision, SIGNAL(triggered()), this, SLOT(ToggleDrawObjectCollision()));
    connect(ui->ActionDrawLights, SIGNAL(triggered()), this, SLOT(ToggleDrawLights()));
    connect(ui->ActionDrawSky, SIGNAL(triggered()), this, SLOT(ToggleDrawSky()));
    connect(ui->ActionGameMode, SIGNAL(triggered()), this, SLOT(ToggleGameMode()));
    connect(ui->ActionDisableAlpha, SIGNAL(triggered()), this, SLOT(ToggleDisableAlpha()));
    connect(ui->ActionNoLighting, SIGNAL(triggered()), this, SLOT(SetNoLighting()));
    connect(ui->ActionBasicLighting, SIGNAL(triggered()), this, SLOT(SetBasicLighting()));
    connect(ui->ActionWorldLighting, SIGNAL(triggered()), this, SLOT(SetWorldLighting()));
    connect(ui->ActionNoBloom, SIGNAL(triggered()), this, SLOT(SetNoBloom()));
    connect(ui->ActionBloomMaps, SIGNAL(triggered()), this, SLOT(SetBloomMaps()));
    connect(ui->ActionFakeBloom, SIGNAL(triggered()), this, SLOT(SetFakeBloom()));
    connect(ui->ActionBloom, SIGNAL(triggered()), this, SLOT(SetBloom()));
    connect(ui->ActionIncrementGizmo, SIGNAL(triggered()), this, SLOT(IncrementGizmo()));
    connect(ui->ActionDecrementGizmo, SIGNAL(triggered()), this, SLOT(DecrementGizmo()));
    connect(ui->ActionCollisionRenderSettings, SIGNAL(triggered()), mpCollisionDialog, SLOT(show()));

    connect(ui->ActionAbout, SIGNAL(triggered(bool)), this, SLOT(About()));
}

CWorldEditor::~CWorldEditor()
{
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
    else return false;
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
    CAssetID AreaID = mpWorld->AreaResourceID(AreaIndex);
    CResourceEntry *pAreaEntry = gpResourceStore->FindEntry(AreaID);
    ASSERT(pAreaEntry);

    mpArea = pAreaEntry->Load();
    ASSERT(mpArea);
    mpWorld->SetAreaLayerInfo(mpArea);
    mScene.SetActiveArea(mpWorld, mpArea);

    // Snap camera to new area
    CCamera *pCamera = &ui->MainViewport->Camera();

    if (pCamera->MoveMode() == ECameraMoveMode::Free)
    {
        CTransform4f AreaTransform = mpArea->Transform();
        CVector3f AreaPosition(AreaTransform[0][3], AreaTransform[1][3], AreaTransform[2][3]);
        pCamera->Snap(AreaPosition);
    }

    UpdateCameraOrbit();

    // Update UI stuff
    UpdateWindowTitle();

    CGameTemplate *pGame = NGameList::GetGameTemplate(mpArea->Game());
    mpLinkDialog->SetGame(pGame);

    QString AreaName = TO_QSTRING(mpWorld->AreaInGameName(AreaIndex));

    if (CurrentGame() < EGame::DKCReturns)
        debugf("Loaded area: %s (%s)", *mpArea->Entry()->Name(), *TO_TSTRING(AreaName));
    else
        debugf("Loaded level: World %s / Area %s (%s)", *mpWorld->Entry()->Name(), *mpArea->Entry()->Name(), *TO_TSTRING(AreaName));

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
    for (CSelectionIterator It(mpSelection); It; ++It)
    {
        if (It->NodeType() == ENodeType::Script)
            return true;
    }

    return false;
}

bool CWorldEditor::IsQuickplayEnabled() const
{
    return mpQuickplayAction->isVisible() && mpQuickplayAction->isEnabled();
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

void CWorldEditor::NotifyNodeAboutToBeDeleted(CSceneNode *pNode)
{
    INodeEditor::NotifyNodeAboutToBeDeleted(pNode);

    if (ui->MainViewport->HoverNode() == pNode)
        ui->MainViewport->ResetHover();
}

bool CWorldEditor::Save()
{
    if (!mpArea)
        return true;

    bool SaveAreaSuccess = mpArea->Entry()->Save();
    bool SaveEGMCSuccess = mpArea->PoiToWorldMap() ? mpArea->PoiToWorldMap()->Entry()->Save() : true;
    bool SaveWorldSuccess = mpWorld->Entry()->Save();

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
    if (!mpSelection->IsEmpty())
    {
        CNodeCopyMimeData *pMimeData = new CNodeCopyMimeData(this);
        qApp->clipboard()->setMimeData(pMimeData);
    }
}

void CWorldEditor::Paste()
{
    if (const CNodeCopyMimeData *pkMimeData =
            qobject_cast<const CNodeCopyMimeData*>(qApp->clipboard()->mimeData()))
    {
        if (pkMimeData->Game() == CurrentGame())
        {
            CVector3f PastePoint;

            if (ui->MainViewport->underMouse() && !ui->MainViewport->IsMouseInputActive())
                PastePoint = ui->MainViewport->HoverPoint();

            else
            {
                CRay Ray = ui->MainViewport->Camera().CastRay(CVector2f(0.f, 0.f));
                SRayIntersection Intersect = ui->MainViewport->SceneRayCast(Ray);

                if (Intersect.Hit)
                    PastePoint = Intersect.HitPoint;
                else
                    PastePoint = Ray.PointOnRay(10.f);
            }

            CPasteNodesCommand *pCmd = new CPasteNodesCommand(this, mpScriptSidebar->CreateTab()->SpawnLayer(), PastePoint);
            UndoStack().push(pCmd);
        }
    }
}

void CWorldEditor::OpenProject()
{
    UICommon::OpenProject();
}

void CWorldEditor::OpenRecentProject()
{
    QAction *pSender = qobject_cast<QAction*>(sender());

    if (pSender)
    {
        QSettings Settings;
        QStringList RecentProjectsList = Settings.value(QStringLiteral("WorldEditor/RecentProjectsList")).toStringList();

        int ProjIndex = pSender->data().toInt();
        QString ProjPath = RecentProjectsList[ProjIndex];
        gpEdApp->OpenProject(ProjPath);
    }
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
    if (ExportDialog.HasValidDisc()) ExportDialog.exec();

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
    ChangeEditMode((EWorldEditorMode) Mode);
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
    CProjectSettingsDialog *pDialog = gpEdApp->ProjectDialog();
    pDialog->show();
    pDialog->raise();
}

void CWorldEditor::OnActiveProjectChanged(CGameProject *pProj)
{
    ui->ActionProjectSettings->setEnabled( pProj != nullptr );
    ui->ActionCloseProject->setEnabled( pProj != nullptr );
    mpPoiMapAction->setVisible( pProj != nullptr && pProj->Game() >= EGame::EchoesDemo && pProj->Game() <= EGame::Corruption );
    mpQuickplayAction->setVisible( pProj != nullptr && NDolphinIntegration::IsQuickplaySupported(pProj) );
    ResetCamera();
    UpdateWindowTitle();

    // Update tweak editor
    // We update this here to ensure we can update the menu item correctly without risking
    // that this function runs before the tweak editor has a chance to update its tweak list.
    mpTweakEditor->OnProjectChanged(pProj);
    ui->ActionEditTweaks->setEnabled( mpTweakEditor->HasTweaks() );

    // Default bloom to Fake Bloom for Metroid Prime 3; disable for other games
    bool AllowBloom = (CurrentGame() == EGame::CorruptionProto || CurrentGame() == EGame::Corruption);
    AllowBloom ? SetFakeBloom() : SetNoBloom();
    ui->menuBloom->setEnabled(AllowBloom);

    // Update recent projects list
    UpdateOpenRecentActions();

    // Reset editor mode
    ChangeEditMode(eWEM_EditWorldInfo);
}

void CWorldEditor::OnLinksModified(const QList<CScriptObject*>& rkInstances)
{
    for (CScriptObject *pInstance : rkInstances)
    {
        CScriptNode *pNode = mScene.NodeForInstance(pInstance);
        pNode->LinksModified();
    }

    if (!rkInstances.isEmpty())
        emit InstanceLinksModified(rkInstances);
}

void CWorldEditor::OnPropertyModified(IProperty *pProp)
{
    bool ShouldUpdateSelection = false;

    for (CSelectionIterator It(mpSelection); It; ++It)
    {
        CSceneNode* pNode = *It;

        if (pNode && pNode->NodeType() == ENodeType::Script)
        {
            CScriptNode* pScript = static_cast<CScriptNode*>(pNode);
            pScript->PropertyModified(pProp);

            // If this is the name, update other parts of the UI to reflect the new value.
            if ( pProp->Name() == "Name" )
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
            CAssetProperty *pAsset = TPropCast<CAssetProperty>(pProp);
            const CResTypeFilter& rkFilter = pAsset->GetTypeFilter();

            if (rkFilter.Accepts(EResourceType::Model) || rkFilter.Accepts(EResourceType::AnimSet) || rkFilter.Accepts(EResourceType::Character))
                ShouldUpdateSelection = true;
        }
        else if (pProp->Type() == EPropertyType::AnimationSet)
            ShouldUpdateSelection = true;
    }

    if (ShouldUpdateSelection)
    {
        SelectionModified();
    }
}

void CWorldEditor::SetSelectionActive(bool Active)
{
    // Gather list of selected objects that actually have Active properties
    QVector<CScriptObject*> Objects;

    for (CSelectionIterator It(mpSelection); It; ++It)
    {
        if (It->NodeType() == ENodeType::Script)
        {
            CScriptNode* pScript = static_cast<CScriptNode*>(*It);
            CScriptObject* pInst = pScript->Instance();
            Objects.push_back(pInst);
        }
    }

    if (!Objects.isEmpty())
    {
        UndoStack().beginMacro(tr("Toggle Active"));

        while (!Objects.isEmpty())
        {
            QVector<CScriptObject*> CommandObjects;
            CScriptTemplate* pTemplate = Objects[0]->Template();
            CBoolProperty* pActiveProperty = pTemplate->ActiveProperty();

            for (int ObjIdx = 0; ObjIdx < Objects.size(); ObjIdx++)
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
                CPropertyModel* pModel = qobject_cast<CPropertyModel*>(
                            mpScriptSidebar->ModifyTab()->PropertyView()->model()
                        );

                CEditScriptPropertyCommand* pCommand = new CEditScriptPropertyCommand(
                            pActiveProperty,
                            CommandObjects,
                            pModel
                        );

                pCommand->SaveOldData();

                for (CScriptObject* pInstance : CommandObjects)
                    pInstance->SetActive(Active);

                pCommand->SaveNewData();

                UndoStack().push(pCommand);
            }
        }

        UndoStack().endMacro();
    }
}

void CWorldEditor::SetSelectionInstanceNames(const QString& rkNewName, bool IsDone)
{
    // todo: this only supports one node at a time because a macro prevents us from merging undo commands
    // this is fine right now because this function is only ever called with a selection of one node, but probably want to fix in the future
    /*if (mpSelection->Size() == 1 && mpSelection->Front()->NodeType() == eScriptNode)
    {
        CScriptNode *pNode = static_cast<CScriptNode*>(mpSelection->Front());
        CScriptObject *pInst = pNode->Instance();

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

    for (CSelectionIterator It(mpSelection); It; ++It)
    {
        if (It->NodeType() == ENodeType::Script)
            ScriptNodes.push_back(static_cast<CScriptNode*>(*It));
    }

    if (!ScriptNodes.isEmpty())
        UndoStack().push(new CChangeLayerCommand(this, ScriptNodes, pLayer));
}

void CWorldEditor::DeleteSelection()
{
    if (HasAnyScriptNodesSelected())
    {
        CDeleteSelectionCommand *pCmd = new CDeleteSelectionCommand(this);
        UndoStack().push(pCmd);
    }
}

void CWorldEditor::UpdateOpenRecentActions()
{
    QSettings Settings;
    QStringList RecentProjectsList = Settings.value(QStringLiteral("WorldEditor/RecentProjectsList")).toStringList();

    // Bump the current project to the front
    CGameProject *pProj = gpEdApp->ActiveProject();

    if (pProj)
    {
        QString ProjPath = TO_QSTRING(pProj->ProjectPath());
        RecentProjectsList.removeAll(ProjPath);
        RecentProjectsList.prepend(ProjPath);
    }

    // Remove projects that don't exist anymore
    for (const QString& rkProj : RecentProjectsList)
    {
        if (!FileUtil::Exists( TO_TSTRING(rkProj) ) || rkProj.contains('\\') )
            RecentProjectsList.removeAll(rkProj);
    }

    Settings.setValue(QStringLiteral("WorldEditor/RecentProjectsList"), RecentProjectsList);

    // Set up the menu actions
    for (int iProj = 0; iProj < mskMaxRecentProjects; iProj++)
    {
        QAction *pAction = mRecentProjectsActions[iProj];

        if (iProj < RecentProjectsList.size())
        {
            QString ActionText = tr("&%1 %2").arg(iProj).arg(RecentProjectsList[iProj]);
            pAction->setText(ActionText);
            pAction->setVisible(true);
        }

        else
            pAction->setVisible(false);
    }
}

void CWorldEditor::UpdateWindowTitle()
{
    QString WindowTitle = QStringLiteral("%APP_FULL_NAME%");
    CGameProject *pProj = gpEdApp->ActiveProject();

    if (pProj)
    {
        WindowTitle += " - " + TO_QSTRING( pProj->Name() );

        if (mpWorld)
        {
            WindowTitle += " - " + TO_QSTRING(mpWorld->InGameName());

            if (mpArea && CurrentGame() < EGame::DKCReturns)
                WindowTitle += " - " + TO_QSTRING( mpWorld->AreaInGameName(mpArea->WorldIndex()) );
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

            default: break;
            }
        }
        else if (!mpSelection->IsEmpty()) SpinBoxValue = mpSelection->Front()->AbsolutePosition();

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

    if (mpSelection->Size() == 1)
        SelectionText = TO_QSTRING(mpSelection->Front()->Name());
    else if (mpSelection->Size() > 1)
        SelectionText = tr("%1 objects selected").arg(mpSelection->Size());

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
        CVector3f Start = mScene.NodeForInstance(mpLinkDialog->Sender())->CenterPoint();
        CVector3f End = mScene.NodeForInstance(mpLinkDialog->Receiver())->CenterPoint();
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
        else if (mpScriptSidebar->ModifyTab()->IsPicking() && mpScriptSidebar->ModifyTab()->EditNode()->NodeType() == ENodeType::Script)
            pSender = static_cast<CScriptNode*>(mpScriptSidebar->ModifyTab()->EditNode())->Instance();

        // No sender and no receiver = no line
        if (!pSender && !pReceiver)
            ui->MainViewport->SetLinkLineEnabled(false);

        // Yes sender and yes receiver = yes line
        else if (pSender && pReceiver)
        {
            ui->MainViewport->SetLinkLineEnabled(true);
            ui->MainViewport->SetLinkLine( mScene.NodeForInstance(pSender)->CenterPoint(), mScene.NodeForInstance(pReceiver)->CenterPoint() );
        }

        // Compensate for missing sender or missing receiver
        else
        {
            bool IsPicking = (mIsMakingLink || mpLinkDialog->IsPicking() || mpScriptSidebar->ModifyTab()->IsPicking());

            if (ui->MainViewport->underMouse() && !ui->MainViewport->IsMouseInputActive() && IsPicking)
            {
                CSceneNode *pHoverNode = ui->MainViewport->HoverNode();
                CScriptObject *pInst = (pSender ? pSender : pReceiver);

                CVector3f Start = mScene.NodeForInstance(pInst)->CenterPoint();
                CVector3f End = (pHoverNode && pHoverNode->NodeType() == ENodeType::Script ? pHoverNode->CenterPoint() : ui->MainViewport->HoverPoint());
                ui->MainViewport->SetLinkLineEnabled(true);
                ui->MainViewport->SetLinkLine(Start, End);
            }

            else
                ui->MainViewport->SetLinkLineEnabled(false);
        }
    }
}

void CWorldEditor::LaunchQuickplay()
{
    CVector3f CameraPosition = Viewport()->Camera().Position();
    LaunchQuickplayFromLocation(CameraPosition, false);
}

void CWorldEditor::LaunchQuickplayFromLocation(CVector3f Location, bool ForceAsSpawnPosition)
{
    // This function should not be called if a level is not open in a project.
    ASSERT( gpEdApp->ActiveProject() != nullptr );
    ASSERT( mpWorld && mpArea );

    // Fill in parameters and start running
    SQuickplayParameters Parameters = mQuickplayParms;
    Parameters.BootWorldAssetID = mpWorld->ID().ToLong();
    Parameters.BootAreaAssetID = mpArea->ID().ToLong();
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
QAction* CWorldEditor::AddEditModeButton(QIcon Icon, QString ToolTip, EWorldEditorMode Mode)
{
    ASSERT(mpEditModeButtonGroup->button(Mode) == nullptr);

    QPushButton* pButton = new QPushButton(Icon, {}, this);
    pButton->setCheckable(true);
    pButton->setToolTip(ToolTip);
    pButton->setIconSize(QSize(24, 24));

    QAction *pAction = ui->EditModeToolBar->addWidget(pButton);
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
    ui->TransformSpinBox->SetSingleStep( (mode == CGizmo::EGizmoMode::Rotate ? 1.0 : 0.1) );
    ui->TransformSpinBox->SetDefaultValue( (mode == CGizmo::EGizmoMode::Scale ? 1.0 : 0.0) );
}

// ************ PRIVATE SLOTS ************
void CWorldEditor::OnClipboardDataModified()
{
    const QMimeData *pkClipboardMimeData = qApp->clipboard()->mimeData();
    const CNodeCopyMimeData *pkMimeData = qobject_cast<const CNodeCopyMimeData*>(pkClipboardMimeData);
    bool ValidMimeData = (pkMimeData && pkMimeData->Game() == CurrentGame());
    ui->ActionPaste->setEnabled(ValidMimeData);
}

void CWorldEditor::OnSelectionModified()
{
    ui->TransformSpinBox->setEnabled(!mpSelection->IsEmpty());

    bool HasScriptNode = HasAnyScriptNodesSelected();
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
        if (It->NodeType() == ENodeType::Script)
            SelectedScriptNodes.push_back(static_cast<CScriptNode*>(*It));
    }

    if (!SelectedScriptNodes.isEmpty())
    {
        CConfirmUnlinkDialog Dialog(this);
        Dialog.exec();

        if (Dialog.UserChoice() != CConfirmUnlinkDialog::EChoice::Cancel)
        {
            UndoStack().beginMacro(tr("Unlink"));
            bool UnlinkIncoming = (Dialog.UserChoice() != CConfirmUnlinkDialog::EChoice::OutgoingOnly);
            bool UnlinkOutgoing = (Dialog.UserChoice() != CConfirmUnlinkDialog::EChoice::IncomingOnly);

            for (CScriptNode *pNode : SelectedScriptNodes)
            {
                CScriptObject *pInst = pNode->Instance();

                if (UnlinkIncoming)
                {
                    QVector<uint32> LinkIndices;
                    for (uint32 iLink = 0; iLink < pInst->NumLinks(ELinkType::Incoming); iLink++)
                        LinkIndices.push_back(iLink);

                    CDeleteLinksCommand *pCmd = new CDeleteLinksCommand(this, pInst, ELinkType::Incoming, LinkIndices);
                    UndoStack().push(pCmd);
                }

                if (UnlinkOutgoing)
                {
                    QVector<uint32> LinkIndices;
                    for (uint32 iLink = 0; iLink < pInst->NumLinks(ELinkType::Outgoing); iLink++)
                        LinkIndices.push_back(iLink);

                    CDeleteLinksCommand *pCmd = new CDeleteLinksCommand(this, pInst, ELinkType::Outgoing, LinkIndices);
                    UndoStack().push(pCmd);
                }
            }

            UndoStack().endMacro();
        }
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

void CWorldEditor::UpdateCameraOrbit()
{
    CCamera *pCamera = &ui->MainViewport->Camera();

    if (!mpSelection->IsEmpty())
        pCamera->SetOrbit(mpSelection->Bounds());
    else if (mpArea)
        pCamera->SetOrbit(mpArea->AABox(), 1.2f);
    else
        pCamera->ResetOrbit();
}

void CWorldEditor::OnCameraSpeedChange(double Speed)
{
    static const double skDefaultSpeed = 1.0;
    ui->MainViewport->Camera().SetMoveSpeed(skDefaultSpeed * Speed);

    ui->CamSpeedSpinBox->blockSignals(true);
    ui->CamSpeedSpinBox->setValue(Speed);
    ui->CamSpeedSpinBox->blockSignals(false);
}

void CWorldEditor::OnTransformSpinBoxModified(CVector3f Value)
{
    if (mpSelection->IsEmpty()) return;

    switch (mGizmo.Mode())
    {
        // Use absolute position/rotation, but relative scale. (This way spinbox doesn't show preview multiplier)
        case CGizmo::EGizmoMode::Translate:
        {
            CVector3f Delta = Value - mpSelection->Front()->AbsolutePosition();
            UndoStack().push(new CTranslateNodeCommand(this, mpSelection->SelectedNodeList(), Delta, mTranslateSpace));
            break;
        }

        case CGizmo::EGizmoMode::Rotate:
        {
            CQuaternion Delta = CQuaternion::FromEuler(Value) * mpSelection->Front()->AbsoluteRotation().Inverse();
            UndoStack().push(new CRotateNodeCommand(this, mpSelection->SelectedNodeList(), true, mGizmo.Position(), mGizmo.Rotation(), Delta, mRotateSpace));
            break;
        }

        case CGizmo::EGizmoMode::Scale:
        {
            CVector3f Delta = Value / mpSelection->Front()->AbsoluteScale();
            UndoStack().push(new CScaleNodeCommand(this, mpSelection->SelectedNodeList(), true, mGizmo.Position(), Delta));
            break;
        }
        default: break;
    }

    UpdateGizmoUI();
}

void CWorldEditor::OnTransformSpinBoxEdited(CVector3f)
{
    if (mpSelection->IsEmpty()) return;

    if (mGizmo.Mode() == CGizmo::EGizmoMode::Translate)   UndoStack().push(CTranslateNodeCommand::End());
    else if (mGizmo.Mode() == CGizmo::EGizmoMode::Rotate) UndoStack().push(CRotateNodeCommand::End());
    else if (mGizmo.Mode() == CGizmo::EGizmoMode::Scale)  UndoStack().push(CScaleNodeCommand::End());

    UpdateGizmoUI();
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
