#include "CModelEditorWindow.h"
#include "ui_CModelEditorWindow.h"
#include "Editor/UICommon.h"
#include "Editor/Widgets/WColorPicker.h"

#include <Common/TString.h>
#include <Core/Render/CDrawUtil.h>
#include <Core/Render/CRenderer.h>
#include <Core/Resource/cooker/CTextureEncoder.h>
#include <Core/Resource/factory/CModelLoader.h>
#include <Core/Resource/factory/CMaterialLoader.h>
#include <Core/Resource/factory/CTextureDecoder.h>
#include <Core/Scene/CScene.h>

#include <iostream>
#include <QFileDialog>
#include <QMessageBox>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

CModelEditorWindow::CModelEditorWindow(CModel *pModel, QWidget *pParent)
    : IEditor(pParent)
    , ui(new Ui::CModelEditorWindow)
    , mpScene(new CScene())
    , mpCurrentMat(nullptr)
    , mpCurrentModel(nullptr)
    , mpCurrentModelNode(new CModelNode(mpScene, -1))
    , mpCurrentPass(nullptr)
    , mIgnoreSignals(false)
{
    ui->setupUi(this);
    ui->ActionSave->setEnabled( pModel->Game() == EGame::Prime ); // we don't support saving games later than MP1
    REPLACE_WINDOWTITLE_APPVARS;

    ui->Viewport->SetNode(mpCurrentModelNode);
    ui->Viewport->SetClearColor(CColor(0.3f, 0.3f, 0.3f, 1.f));

    CCamera& rCamera = ui->Viewport->Camera();
    rCamera.Snap(CVector3f(0, 3, 1));
    rCamera.SetMoveMode(eOrbitCamera);
    rCamera.SetOrbit(CVector3f(0, 0, 1), 3.f);
    rCamera.SetMoveSpeed(0.5f);

    // UI initialization
    UpdateAnimParamUI(-1);
    ui->IndTextureResSelector->SetTypeFilter(pModel->Game(), "TXTR");
    ui->PassTextureResSelector->SetTypeFilter(pModel->Game(), "TXTR");
    ui->PassTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->PassTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->ClearColorPicker->SetColor(QColor(76, 76, 76, 255));

    // Editor UI Signal/Slot setup
    ui->SetSelectionComboBox->setProperty          ("ModelEditorWidgetType", eSetSelectComboBox);
    ui->MatSelectionComboBox->setProperty          ("ModelEditorWidgetType", eMatSelectComboBox);
    ui->EnableTransparencyCheck->setProperty       ("ModelEditorWidgetType", eEnableTransparencyCheckBox);
    ui->EnablePunchthroughCheck->setProperty       ("ModelEditorWidgetType", eEnablePunchthroughCheckBox);
    ui->EnableReflectionCheck->setProperty         ("ModelEditorWidgetType", eEnableReflectionCheckBox);
    ui->EnableSurfaceReflectionCheck->setProperty  ("ModelEditorWidgetType", eEnableSurfaceReflectionCheckBox);
    ui->EnableDepthWriteCheck->setProperty         ("ModelEditorWidgetType", eEnableDepthWriteCheckBox);
    ui->EnableOccluderCheck->setProperty           ("ModelEditorWidgetType", eEnableOccluderCheckBox);
    ui->EnableLightmapCheck->setProperty           ("ModelEditorWidgetType", eEnableLightmapCheckBox);
    ui->EnableDynamicLightingCheck->setProperty    ("ModelEditorWidgetType", eEnableLightingCheckBox);
    ui->SourceBlendComboBox->setProperty           ("ModelEditorWidgetType", eSourceBlendComboBox);
    ui->DestBlendComboBox->setProperty             ("ModelEditorWidgetType", eDestBlendComboBox);
    ui->KonstColorPickerA->setProperty             ("ModelEditorWidgetType", eKonstColorPickerA);
    ui->KonstColorPickerB->setProperty             ("ModelEditorWidgetType", eKonstColorPickerB);
    ui->KonstColorPickerC->setProperty             ("ModelEditorWidgetType", eKonstColorPickerC);
    ui->KonstColorPickerD->setProperty             ("ModelEditorWidgetType", eKonstColorPickerD);
    ui->IndTextureResSelector->setProperty         ("ModelEditorWidgetType", eIndTextureResSelector);
    ui->PassTable->setProperty                     ("ModelEditorWidgetType", ePassTableWidget);
    ui->TevKColorSelComboBox->setProperty          ("ModelEditorWidgetType", eTevKColorSelComboBox);
    ui->TevKAlphaSelComboBox->setProperty          ("ModelEditorWidgetType", eTevKAlphaSelComboBox);
    ui->TevRasSelComboBox->setProperty             ("ModelEditorWidgetType", eTevRasSelComboBox);
    ui->TexCoordSrcComboBox->setProperty           ("ModelEditorWidgetType", eTevTexSourceComboBox);
    ui->PassTextureResSelector->setProperty        ("ModelEditorWidgetType", ePassTextureResSelector);
    ui->TevColor1ComboBox->setProperty             ("ModelEditorWidgetType", eTevColorComboBoxA);
    ui->TevColor2ComboBox->setProperty             ("ModelEditorWidgetType", eTevColorComboBoxB);
    ui->TevColor3ComboBox->setProperty             ("ModelEditorWidgetType", eTevColorComboBoxC);
    ui->TevColor4ComboBox->setProperty             ("ModelEditorWidgetType", eTevColorComboBoxD);
    ui->TevColorOutputComboBox->setProperty        ("ModelEditorWidgetType", eTevColorOutputComboBox);
    ui->TevAlpha1ComboBox->setProperty             ("ModelEditorWidgetType", eTevAlphaComboBoxA);
    ui->TevAlpha2ComboBox->setProperty             ("ModelEditorWidgetType", eTevAlphaComboBoxB);
    ui->TevAlpha3ComboBox->setProperty             ("ModelEditorWidgetType", eTevAlphaComboBoxC);
    ui->TevAlpha4ComboBox->setProperty             ("ModelEditorWidgetType", eTevAlphaComboBoxD);
    ui->TevAlphaOutputComboBox->setProperty        ("ModelEditorWidgetType", eTevAlphaOutputComboBox);
    ui->AnimTypeComboBox->setProperty              ("ModelEditorWidgetType", eAnimModeComboBox);
    ui->AnimParamASpinBox->setProperty             ("ModelEditorWidgetType", eAnimParamASpinBox);
    ui->AnimParamBSpinBox->setProperty             ("ModelEditorWidgetType", eAnimParamBSpinBox);
    ui->AnimParamCSpinBox->setProperty             ("ModelEditorWidgetType", eAnimParamCSpinBox);
    ui->AnimParamDSpinBox->setProperty             ("ModelEditorWidgetType", eAnimParamDSpinBox);

    connect(ui->ActionImport, SIGNAL(triggered()), this, SLOT(Import()));
    connect(ui->ActionSave, SIGNAL(triggered()), this, SLOT(Save()));
    connect(ui->ActionConvertToDDS, SIGNAL(triggered()), this, SLOT(ConvertToDDS()));
    connect(ui->ActionConvertToTXTR, SIGNAL(triggered()), this, SLOT(ConvertToTXTR()));
    connect(ui->MeshPreviewButton, SIGNAL(clicked()), this, SLOT(SetMeshPreview()));
    connect(ui->SpherePreviewButton, SIGNAL(clicked()), this, SLOT(SetSpherePreview()));
    connect(ui->FlatPreviewButton, SIGNAL(clicked()), this, SLOT(SetFlatPreview()));
    connect(ui->ClearColorPicker, SIGNAL(ColorChanged(QColor)), this, SLOT(ClearColorChanged(QColor)));
    connect(ui->CameraModeButton, SIGNAL(clicked()), this, SLOT(ToggleCameraMode()));
    connect(ui->ToggleGridButton, SIGNAL(toggled(bool)), this, SLOT(ToggleGrid(bool)));

    connect(ui->SetSelectionComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(UpdateUI(int)));
    connect(ui->MatSelectionComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(UpdateUI(int)));
    connect(ui->EnableTransparencyCheck,      SIGNAL(toggled(bool)), this, SLOT(UpdateMaterial(bool)));
    connect(ui->EnablePunchthroughCheck,      SIGNAL(toggled(bool)), this, SLOT(UpdateMaterial(bool)));
    connect(ui->EnableReflectionCheck,        SIGNAL(toggled(bool)), this, SLOT(UpdateMaterial(bool)));
    connect(ui->EnableSurfaceReflectionCheck, SIGNAL(toggled(bool)), this, SLOT(UpdateMaterial(bool)));
    connect(ui->EnableDepthWriteCheck,        SIGNAL(toggled(bool)), this, SLOT(UpdateMaterial(bool)));
    connect(ui->EnableOccluderCheck,          SIGNAL(toggled(bool)), this, SLOT(UpdateMaterial(bool)));
    connect(ui->EnableLightmapCheck,          SIGNAL(toggled(bool)), this, SLOT(UpdateMaterial(bool)));
    connect(ui->EnableDynamicLightingCheck,   SIGNAL(toggled(bool)), this, SLOT(UpdateMaterial(bool)));
    connect(ui->SourceBlendComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(UpdateMaterial(int)));
    connect(ui->DestBlendComboBox,   SIGNAL(currentIndexChanged(int)), this, SLOT(UpdateMaterial(int)));
    connect(ui->KonstColorPickerA, SIGNAL(ColorChanged(QColor)), this, SLOT(UpdateMaterial(QColor)));
    connect(ui->KonstColorPickerB, SIGNAL(ColorChanged(QColor)), this, SLOT(UpdateMaterial(QColor)));
    connect(ui->KonstColorPickerC, SIGNAL(ColorChanged(QColor)), this, SLOT(UpdateMaterial(QColor)));
    connect(ui->KonstColorPickerD, SIGNAL(ColorChanged(QColor)), this, SLOT(UpdateMaterial(QColor)));
    connect(ui->PassTable, SIGNAL(cellClicked(int,int)), this, SLOT(UpdateMaterial(int, int)));
    connect(ui->TevKColorSelComboBox,   SIGNAL(currentIndexChanged(int)), this, SLOT(UpdateMaterial(int)));
    connect(ui->TevKAlphaSelComboBox,   SIGNAL(currentIndexChanged(int)), this, SLOT(UpdateMaterial(int)));
    connect(ui->TevRasSelComboBox,      SIGNAL(currentIndexChanged(int)), this, SLOT(UpdateMaterial(int)));
    connect(ui->TexCoordSrcComboBox,    SIGNAL(currentIndexChanged(int)), this, SLOT(UpdateMaterial(int)));
    connect(ui->PassTextureResSelector, SIGNAL(ResourceChanged(QString)), this, SLOT(UpdateMaterial(QString)));
    connect(ui->TevColor1ComboBox,      SIGNAL(currentIndexChanged(int)), this, SLOT(UpdateMaterial(int)));
    connect(ui->TevColor2ComboBox,      SIGNAL(currentIndexChanged(int)), this, SLOT(UpdateMaterial(int)));
    connect(ui->TevColor3ComboBox,      SIGNAL(currentIndexChanged(int)), this, SLOT(UpdateMaterial(int)));
    connect(ui->TevColor4ComboBox,      SIGNAL(currentIndexChanged(int)), this, SLOT(UpdateMaterial(int)));
    connect(ui->TevColorOutputComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(UpdateMaterial(int)));
    connect(ui->TevAlpha1ComboBox,      SIGNAL(currentIndexChanged(int)), this, SLOT(UpdateMaterial(int)));
    connect(ui->TevAlpha2ComboBox,      SIGNAL(currentIndexChanged(int)), this, SLOT(UpdateMaterial(int)));
    connect(ui->TevAlpha3ComboBox,      SIGNAL(currentIndexChanged(int)), this, SLOT(UpdateMaterial(int)));
    connect(ui->TevAlpha4ComboBox,      SIGNAL(currentIndexChanged(int)), this, SLOT(UpdateMaterial(int)));
    connect(ui->TevAlphaOutputComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(UpdateMaterial(int)));
    connect(ui->AnimTypeComboBox,       SIGNAL(currentIndexChanged(int)), this, SLOT(UpdateMaterial(int)));
    connect(ui->AnimParamASpinBox, SIGNAL(valueChanged(double)), this, SLOT(UpdateMaterial(double)));
    connect(ui->AnimParamBSpinBox, SIGNAL(valueChanged(double)), this, SLOT(UpdateMaterial(double)));
    connect(ui->AnimParamCSpinBox, SIGNAL(valueChanged(double)), this, SLOT(UpdateMaterial(double)));
    connect(ui->AnimParamDSpinBox, SIGNAL(valueChanged(double)), this, SLOT(UpdateMaterial(double)));
    // That was fun

    SetActiveModel(pModel);
}

CModelEditorWindow::~CModelEditorWindow()
{
    delete mpCurrentModelNode;
    delete mpScene;
    delete ui;
}

void CModelEditorWindow::RefreshViewport()
{
    ui->Viewport->ProcessInput();
    ui->Viewport->Render();
}

void CModelEditorWindow::SetActiveModel(CModel *pModel)
{
    mpCurrentModelNode->SetModel(pModel);
    mpCurrentModel = pModel;
    ui->Viewport->Camera().SetOrbit(pModel->AABox());

    uint32 NumVertices = (pModel ? pModel->GetVertexCount() : 0);
    uint32 NumTriangles = (pModel ? pModel->GetTriangleCount() : 0);
    uint32 NumMats = (pModel ? pModel->GetMatCount() : 0);
    uint32 NumMatSets = (pModel ? pModel->GetMatSetCount() : 0);
    ui->MeshInfoLabel->setText(QString::number(NumVertices) + " vertices, " + QString::number(NumTriangles) + " triangles");
    ui->MatInfoLabel->setText(QString::number(NumMats) + " materials, " + QString::number(NumMatSets) + " set" + (NumMatSets == 1 ? "" : "s"));

    // Set items in matset combo box
    ui->SetSelectionComboBox->blockSignals(true);
    ui->SetSelectionComboBox->clear();

    for (uint32 iSet = 0; iSet < NumMatSets; iSet++)
        ui->SetSelectionComboBox->addItem("Set #" + QString::number(iSet + 1));

    ui->SetSelectionComboBox->setCurrentIndex(0);
    ui->SetSelectionComboBox->blockSignals(false);

    // Set items in mat combo box
    ui->MatSelectionComboBox->blockSignals(true);
    ui->MatSelectionComboBox->clear();

    for (uint32 iMat = 0; iMat < NumMats; iMat++)
    {
        TString MatName = pModel->GetMaterialByIndex(0, iMat)->Name();
        ui->MatSelectionComboBox->addItem(TO_QSTRING(MatName));
    }

    ui->MatSelectionComboBox->setCurrentIndex(0);
    ui->MatSelectionComboBox->setEnabled( NumMats > 1 );
    ui->MatSelectionComboBox->blockSignals(false);

    // Emit signals to set up UI
    ui->SetSelectionComboBox->currentIndexChanged(0);

    // Gray out set selection for models with one set
    ui->SetSelectionComboBox->setEnabled( NumMatSets > 1 );
    ui->MatSelectionComboBox->setEnabled( NumMats > 1 );
}

void CModelEditorWindow::SetActiveMaterial(int MatIndex)
{
    if (!mpCurrentModel) return;

    uint32 SetIndex = ui->SetSelectionComboBox->currentIndex();
    mpCurrentMat = mpCurrentModel->GetMaterialByIndex(SetIndex, MatIndex);
    ui->Viewport->SetActiveMaterial(mpCurrentMat);
    if (!mpCurrentMat) return;

    // Set up UI
    CMaterial::FMaterialOptions Settings = mpCurrentMat->Options();

    mIgnoreSignals = true;
    ui->EnableTransparencyCheck->setChecked( Settings & CMaterial::eTransparent );
    ui->EnablePunchthroughCheck->setChecked( Settings & CMaterial::ePunchthrough );
    ui->EnableReflectionCheck->setChecked( Settings & CMaterial::eReflection );
    ui->EnableSurfaceReflectionCheck->setChecked( Settings & CMaterial::eSurfaceReflection );
    ui->EnableDepthWriteCheck->setChecked( Settings & CMaterial::eDepthWrite );
    ui->EnableOccluderCheck->setChecked( Settings & CMaterial::eOccluder );
    ui->EnableLightmapCheck->setChecked( Settings & CMaterial::eLightmap );
    ui->EnableDynamicLightingCheck->setChecked( mpCurrentMat->IsLightingEnabled() );

    uint32 SrcFac = (uint32) mpCurrentMat->BlendSrcFac();
    uint32 DstFac = (uint32) mpCurrentMat->BlendDstFac();
    if (SrcFac >= 0x300) SrcFac -= 0x2FE;
    if (DstFac >= 0x300) DstFac -= 0x2FE;
    ui->SourceBlendComboBox->setCurrentIndex(SrcFac);
    ui->DestBlendComboBox->setCurrentIndex(DstFac);

    if (Settings & CMaterial::eIndStage)
        ui->IndTextureResSelector->SetResource(mpCurrentMat->IndTexture());
    else
        ui->IndTextureResSelector->Clear();

    for (uint32 iKonst = 0; iKonst < 4; iKonst++)
    {
        QColor Color;
        CColor KColor = mpCurrentMat->Konst(iKonst);
        Color.setRed(KColor.R * 255);
        Color.setGreen(KColor.G * 255);
        Color.setBlue(KColor.B * 255);
        Color.setAlpha(KColor.A * 255);

        if (iKonst == 0) ui->KonstColorPickerA->SetColor(Color);
        else if (iKonst == 1) ui->KonstColorPickerB->SetColor(Color);
        else if (iKonst == 2) ui->KonstColorPickerC->SetColor(Color);
        else if (iKonst == 3) ui->KonstColorPickerD->SetColor(Color);
    }

    uint32 PassCount = mpCurrentMat->PassCount();
    ui->PassTable->clear();
    ui->PassTable->setRowCount(PassCount);

    for (uint32 iPass = 0; iPass < PassCount; iPass++)
    {
        CMaterialPass *pPass = mpCurrentMat->Pass(iPass);

        QTableWidgetItem *pItemA = new QTableWidgetItem("Pass #" + QString::number(iPass + 1) + ": " + TO_QSTRING(pPass->NamedType()));
        QTableWidgetItem *pItemB = new QTableWidgetItem();

        if (pPass->IsEnabled())
            pItemB->setIcon(QIcon(":/icons/Show.png"));
        else
            pItemB->setIcon(QIcon(":/icons/Hide.png"));

        ui->PassTable->setItem(iPass, 0, pItemA);
        ui->PassTable->setItem(iPass, 1, pItemB);
    }

    // Set up the tex coord source combo box so it only shows vertex attributes that exist on this material
    ui->TexCoordSrcComboBox->clear();
    FVertexDescription Desc = mpCurrentMat->VtxDesc();

    ui->TexCoordSrcComboBox->addItem("None");
    if (Desc & ePosition) ui->TexCoordSrcComboBox->addItem("Position");
    if (Desc & eNormal)   ui->TexCoordSrcComboBox->addItem("Normal");
    if (Desc & eTex0)     ui->TexCoordSrcComboBox->addItem("Tex Coord 1");
    if (Desc & eTex1)     ui->TexCoordSrcComboBox->addItem("Tex Coord 2");
    if (Desc & eTex2)     ui->TexCoordSrcComboBox->addItem("Tex Coord 3");
    if (Desc & eTex3)     ui->TexCoordSrcComboBox->addItem("Tex Coord 4");
    if (Desc & eTex4)     ui->TexCoordSrcComboBox->addItem("Tex Coord 5");
    if (Desc & eTex5)     ui->TexCoordSrcComboBox->addItem("Tex Coord 6");
    if (Desc & eTex6)     ui->TexCoordSrcComboBox->addItem("Tex Coord 7");
    if (Desc & eTex7)     ui->TexCoordSrcComboBox->addItem("Tex Coord 8");

    // Emit signal from Pass Table to set up the Pass UI
    mIgnoreSignals = false;

    if (PassCount > 0)
        ui->PassTable->cellClicked(0,0);

    // Activate UI
    ActivateMatEditUI(true);
}

void CModelEditorWindow::SetActivePass(int PassIndex)
{
    // Some modifications have to be made to the values to match GX enums with combo box indices
    mIgnoreSignals = true;
    mpCurrentPass = mpCurrentMat->Pass(PassIndex);

    uint32 KColor = mpCurrentPass->KColorSel();
    uint32 KAlpha = mpCurrentPass->KAlphaSel();
    uint32 Ras = mpCurrentPass->RasSel();
    uint32 TexCoordSrc = mpCurrentPass->TexCoordSource();
    if (KColor >= 0xC) KColor -= 4;
    if (KAlpha >= 0x10) KAlpha -= 8;
    if (Ras == 0xFF) Ras = 7;
    if (TexCoordSrc == 0xFF) TexCoordSrc = 0;
    else TexCoordSrc++;
    if (TexCoordSrc >= 5) TexCoordSrc -= 2;

    ui->PassTextureResSelector->SetResource(mpCurrentPass->Texture());
    ui->TevKColorSelComboBox->setCurrentIndex(KColor);
    ui->TevKAlphaSelComboBox->setCurrentIndex(KAlpha);
    ui->TevRasSelComboBox->setCurrentIndex(Ras);
    ui->TexCoordSrcComboBox->setCurrentIndex(TexCoordSrc);
    ui->TevColor1ComboBox->setCurrentIndex(mpCurrentPass->ColorInput(0));
    ui->TevColor2ComboBox->setCurrentIndex(mpCurrentPass->ColorInput(1));
    ui->TevColor3ComboBox->setCurrentIndex(mpCurrentPass->ColorInput(2));
    ui->TevColor4ComboBox->setCurrentIndex(mpCurrentPass->ColorInput(3));
    ui->TevColorOutputComboBox->setCurrentIndex(mpCurrentPass->ColorOutput());
    ui->TevAlpha1ComboBox->setCurrentIndex(mpCurrentPass->AlphaInput(0));
    ui->TevAlpha2ComboBox->setCurrentIndex(mpCurrentPass->AlphaInput(1));
    ui->TevAlpha3ComboBox->setCurrentIndex(mpCurrentPass->AlphaInput(2));
    ui->TevAlpha4ComboBox->setCurrentIndex(mpCurrentPass->AlphaInput(3));
    ui->TevAlphaOutputComboBox->setCurrentIndex(mpCurrentPass->AlphaOutput());

    int32 AnimMode = mpCurrentPass->AnimMode();
    ui->AnimTypeComboBox->setCurrentIndex(AnimMode + 1);
    UpdateAnimParamUI(AnimMode);

    mIgnoreSignals = false;
}

void CModelEditorWindow::UpdateMaterial()
{
    // This function takes input from buttons
    if (!mpCurrentMat) return;
    if (mIgnoreSignals) return;

    /*EModelEditorWidget Widget = (EModelEditorWidget) sender()->property("ModelEditorWidgetType").toInt();

    switch (Widget)
    {
    case eAddPassButton:
        break;

    case eDeletePassButton:
        break;
    }*/
}

void CModelEditorWindow::UpdateMaterial(int Value)
{
    // This function takes input from combo boxes
    if (!mpCurrentMat) return;
    if (mIgnoreSignals) return;

    EModelEditorWidget Widget = (EModelEditorWidget) sender()->property("ModelEditorWidgetType").toInt();

    switch (Widget)
    {

    case eSourceBlendComboBox:
    case eDestBlendComboBox:
    {
        GLenum SourceFac = ui->SourceBlendComboBox->currentIndex();
        GLenum DstFac = ui->DestBlendComboBox->currentIndex();
        if (SourceFac > 1) SourceFac += 0x2FE;
        if (DstFac > 1) DstFac += 0x2FE;
        mpCurrentMat->SetBlendMode(SourceFac, DstFac);
        break;
    }

    case eTevKColorSelComboBox:
        if (Value >= 8) Value += 4;
        mpCurrentPass->SetKColorSel((ETevKSel) Value);
        break;

    case eTevKAlphaSelComboBox:
        if (Value >= 8) Value += 8;
        mpCurrentPass->SetKAlphaSel((ETevKSel) Value);
        break;

    case eTevRasSelComboBox:
        if (Value == 7) Value = eRasColorNull;
        mpCurrentPass->SetRasSel((ETevRasSel) Value);
        break;

    case eTevTexSelComboBox:
        // todo
        break;

    case eTevTexSourceComboBox:
        if (Value >= 3) Value ++;
        else Value--;
        mpCurrentPass->SetTexCoordSource(Value);
        break;

    case eTevColorComboBoxA:
    case eTevColorComboBoxB:
    case eTevColorComboBoxC:
    case eTevColorComboBoxD:
    {
        ETevColorInput A = (ETevColorInput) ui->TevColor1ComboBox->currentIndex();
        ETevColorInput B = (ETevColorInput) ui->TevColor2ComboBox->currentIndex();
        ETevColorInput C = (ETevColorInput) ui->TevColor3ComboBox->currentIndex();
        ETevColorInput D = (ETevColorInput) ui->TevColor4ComboBox->currentIndex();
        mpCurrentPass->SetColorInputs(A, B, C, D);
        break;
    }

    case eTevColorOutputComboBox:
        mpCurrentPass->SetColorOutput((ETevOutput) Value);
        break;

    case eTevAlphaComboBoxA:
    case eTevAlphaComboBoxB:
    case eTevAlphaComboBoxC:
    case eTevAlphaComboBoxD:
    {
        ETevAlphaInput A = (ETevAlphaInput) ui->TevAlpha1ComboBox->currentIndex();
        ETevAlphaInput B = (ETevAlphaInput) ui->TevAlpha2ComboBox->currentIndex();
        ETevAlphaInput C = (ETevAlphaInput) ui->TevAlpha3ComboBox->currentIndex();
        ETevAlphaInput D = (ETevAlphaInput) ui->TevAlpha4ComboBox->currentIndex();
        mpCurrentPass->SetAlphaInputs(A, B, C, D);
        break;
    }

    case eTevAlphaOutputComboBox:
        mpCurrentPass->SetAlphaOutput((ETevOutput) Value);
        break;

    case eAnimModeComboBox:
        mpCurrentPass->SetAnimMode((EUVAnimMode) (Value - 1));
        UpdateAnimParamUI(Value - 1);
        break;
    }

    mpCurrentMat->GenerateShader();
}

void CModelEditorWindow::UpdateMaterial(int ValueA, int ValueB)
{
    // This function takes input from PassTable
    if (!mpCurrentMat) return;
    if (mIgnoreSignals) return;

    // Select Pass
    if (ValueB == 0)
    {
        SetActivePass(ValueA);
        ui->PassTable->setSelectionMode(QAbstractItemView::SingleSelection);
        ui->PassTable->selectRow(ValueA);
        ui->PassTable->setSelectionMode(QAbstractItemView::NoSelection);
    }

    // Show/Hide Pass
    else if (ValueB == 1)
    {
        bool Enabled = !mpCurrentMat->Pass(ValueA)->IsEnabled();
        mpCurrentMat->Pass(ValueA)->SetEnabled(Enabled);

        if (Enabled)
            ui->PassTable->item(ValueA, ValueB)->setIcon(QIcon(":/icons/Show.png"));
        else
            ui->PassTable->item(ValueA, ValueB)->setIcon(QIcon(":/icons/Hide.png"));
    }
}

void CModelEditorWindow::UpdateMaterial(double Value)
{
    // This function takes input from WDraggableSpinBoxes
    if (!mpCurrentMat) return;
    if (mIgnoreSignals) return;

    EModelEditorWidget Widget = (EModelEditorWidget) sender()->property("ModelEditorWidgetType").toInt();

    switch (Widget)
    {
    case eAnimParamASpinBox:
        mpCurrentPass->SetAnimParam(0, (float) Value);
        break;
    case eAnimParamBSpinBox:
        mpCurrentPass->SetAnimParam(1, (float) Value);
        break;
    case eAnimParamCSpinBox:
        mpCurrentPass->SetAnimParam(2, (float) Value);
        break;
    case eAnimParamDSpinBox:
        mpCurrentPass->SetAnimParam(3, (float) Value);
        break;
    }
}

void CModelEditorWindow::UpdateMaterial(bool Value)
{
    // This function takes input from checkboxes
    if (!mpCurrentMat) return;
    if (mIgnoreSignals) return;

    EModelEditorWidget Widget = (EModelEditorWidget) sender()->property("ModelEditorWidgetType").toInt();

    switch (Widget)
    {

    case eEnableTransparencyCheckBox:
    case eEnablePunchthroughCheckBox:
    case eEnableReflectionCheckBox:
    case eEnableSurfaceReflectionCheckBox:
    case eEnableDepthWriteCheckBox:
    case eEnableOccluderCheckBox:
    case eEnableLightmapCheckBox:
    {
        CMaterial::FMaterialOptions Options = (mpCurrentMat->Options() & 0x2408);
        Options |= (ui->EnableTransparencyCheck->isChecked()      <<  4);
        Options |= (ui->EnablePunchthroughCheck->isChecked()      <<  5);
        Options |= (ui->EnableReflectionCheck->isChecked()        <<  6);
        Options |= (ui->EnableDepthWriteCheck->isChecked()        <<  7);
        Options |= (ui->EnableSurfaceReflectionCheck->isChecked() <<  8);
        Options |= (ui->EnableOccluderCheck->isChecked()          <<  9);
        Options |= (ui->EnableLightmapCheck->isChecked()          << 11);
        mpCurrentMat->SetOptions(Options);
        break;
    }

    case eEnableLightingCheckBox:
        mpCurrentMat->SetLightingEnabled(Value);
        break;

    }
}

void CModelEditorWindow::UpdateMaterial(QColor Color)
{
    // This function takes input from WColorPickers
    if (!mpCurrentMat) return;
    if (mIgnoreSignals) return;

    EModelEditorWidget Widget = (EModelEditorWidget) sender()->property("ModelEditorWidgetType").toInt();
    CColor KColor(Color.red() / 255.f, Color.green() / 255.f, Color.blue() / 255.f, Color.alpha() / 255.f);

    switch (Widget)
    {
    case eKonstColorPickerA:
        mpCurrentMat->SetKonst(KColor, 0);
        break;
    case eKonstColorPickerB:
        mpCurrentMat->SetKonst(KColor, 1);
        break;
    case eKonstColorPickerC:
        mpCurrentMat->SetKonst(KColor, 2);
        break;
    case eKonstColorPickerD:
        mpCurrentMat->SetKonst(KColor, 3);
        break;
    }
}

void CModelEditorWindow::UpdateMaterial(QString Value)
{
    // This function takes input from WResourceSelectors
    if (!mpCurrentMat) return;
    if (mIgnoreSignals) return;

    EModelEditorWidget Widget = (EModelEditorWidget) sender()->property("ModelEditorWidgetType").toInt();
    TResPtr<CTexture> pTex = gpResourceStore->LoadResource(TO_TSTRING(Value));
    if (pTex->Type() != eTexture) pTex = nullptr;

    switch (Widget)
    {
    case ePassTextureResSelector:
        mpCurrentPass->SetTexture(pTex);
        break;

    case eIndTextureResSelector:
        mpCurrentMat->SetIndTexture(pTex);
        break;
    }
}
void CModelEditorWindow::UpdateUI(int Value)
{
    EModelEditorWidget Widget = (EModelEditorWidget) sender()->property("ModelEditorWidgetType").toInt();

    switch (Widget)
    {

    case eSetSelectComboBox:
        mpCurrentModelNode->SetMatSet(Value);
        SetActiveMaterial(ui->MatSelectionComboBox->currentIndex());
        break;

    case eMatSelectComboBox:
        SetActiveMaterial(Value);
        ActivateMatEditUI(true);
        break;
    }
}

// ************ PRIVATE ************
void CModelEditorWindow::ActivateMatEditUI(bool Active)
{
    ui->MatSelectionComboBox->setEnabled(Active);
    ui->GeneralGroupBox->setEnabled(Active);
    ui->PassGroupBox->setEnabled(Active);
}

void CModelEditorWindow::RefreshMaterial()
{
    if (mpCurrentMat) mpCurrentMat->GenerateShader();
}

void CModelEditorWindow::UpdateAnimParamUI(int Mode)
{
    // Update the param labels with actual names + hide unused params for each mode.

    switch (Mode)
    {
    case -1: // N/A
    case 0: // ModelView No Translate
    case 1: // ModelView
    case 6: // Model
        ui->AnimParamALabel->hide();
        ui->AnimParamBLabel->hide();
        ui->AnimParamCLabel->hide();
        ui->AnimParamDLabel->hide();
        ui->AnimParamASpinBox->hide();
        ui->AnimParamBSpinBox->hide();
        ui->AnimParamCSpinBox->hide();
        ui->AnimParamDSpinBox->hide();
        break;

    case 2: // UV Scroll
        ui->AnimParamALabel->setText("<b>Horizontal Offset:</b>");
        ui->AnimParamBLabel->setText("<b>Vertical Offset:</b>");
        ui->AnimParamCLabel->setText("<b>Horizontal Scale:</b>");
        ui->AnimParamDLabel->setText("<b>Vertical Scale:</b>");
        ui->AnimParamASpinBox->setValue(mpCurrentPass->AnimParam(0));
        ui->AnimParamBSpinBox->setValue(mpCurrentPass->AnimParam(1));
        ui->AnimParamCSpinBox->setValue(mpCurrentPass->AnimParam(2));
        ui->AnimParamDSpinBox->setValue(mpCurrentPass->AnimParam(3));
        ui->AnimParamALabel->show();
        ui->AnimParamBLabel->show();
        ui->AnimParamCLabel->show();
        ui->AnimParamDLabel->show();
        ui->AnimParamASpinBox->show();
        ui->AnimParamBSpinBox->show();
        ui->AnimParamCSpinBox->show();
        ui->AnimParamDSpinBox->show();
        break;

    case 3: // Rotation
        ui->AnimParamALabel->setText("<b>Offset:</b>");
        ui->AnimParamBLabel->setText("<b>Scale:</b>");
        ui->AnimParamASpinBox->setValue(mpCurrentPass->AnimParam(0));
        ui->AnimParamBSpinBox->setValue(mpCurrentPass->AnimParam(1));
        ui->AnimParamALabel->show();
        ui->AnimParamBLabel->show();
        ui->AnimParamCLabel->hide();
        ui->AnimParamDLabel->hide();
        ui->AnimParamASpinBox->show();
        ui->AnimParamBSpinBox->show();
        ui->AnimParamCSpinBox->hide();
        ui->AnimParamDSpinBox->hide();
        break;

    case 4: // Horizontal Filmstrip
    case 5: // Vertical Filmstrip
        ui->AnimParamALabel->setText("<b>Scale:</b>");
        ui->AnimParamBLabel->setText("<b>Num Frames:</b>");
        ui->AnimParamCLabel->setText("<b>Step:</b>");
        ui->AnimParamDLabel->setText("<b>Time Offset:</bB>");
        ui->AnimParamASpinBox->setValue(mpCurrentPass->AnimParam(0));
        ui->AnimParamBSpinBox->setValue(mpCurrentPass->AnimParam(1));
        ui->AnimParamCSpinBox->setValue(mpCurrentPass->AnimParam(2));
        ui->AnimParamDSpinBox->setValue(mpCurrentPass->AnimParam(3));
        ui->AnimParamALabel->show();
        ui->AnimParamBLabel->show();
        ui->AnimParamCLabel->show();
        ui->AnimParamDLabel->show();
        ui->AnimParamASpinBox->show();
        ui->AnimParamBSpinBox->show();
        ui->AnimParamCSpinBox->show();
        ui->AnimParamDSpinBox->show();
        break;

    case 7: // Mysterious mode 7
        ui->AnimParamALabel->setText("<b>ParamA:</b>");
        ui->AnimParamBLabel->setText("<b>ParamB:</b>");
        ui->AnimParamASpinBox->setValue(mpCurrentPass->AnimParam(0));
        ui->AnimParamBSpinBox->setValue(mpCurrentPass->AnimParam(1));
        ui->AnimParamALabel->show();
        ui->AnimParamBLabel->show();
        ui->AnimParamCLabel->hide();
        ui->AnimParamDLabel->hide();
        ui->AnimParamASpinBox->show();
        ui->AnimParamBSpinBox->show();
        ui->AnimParamCSpinBox->hide();
        ui->AnimParamDSpinBox->hide();
        break;
    }
}

void CModelEditorWindow::Import()
{
    QString FileName = QFileDialog::getOpenFileName(this, "Model", "", "*.obj;*.fbx;*.dae;*.3ds;*.blend");
    if (FileName.isEmpty()) return;

    Assimp::Importer Importer;
    Importer.SetPropertyInteger(AI_CONFIG_PP_FD_REMOVE, 1);
    Importer.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS,
                                aiComponent_TANGENTS_AND_BITANGENTS |
                                aiComponent_ANIMATIONS |
                                aiComponent_LIGHTS |
                                aiComponent_CAMERAS);

    const aiScene *pScene = Importer.ReadFile(FileName.toStdString(),
                                              aiProcess_JoinIdenticalVertices |
                                              aiProcess_Triangulate |
                                              aiProcess_RemoveComponent |
                                              //aiProcess_GenSmoothNormals |
                                              //aiProcess_SplitLargeMeshes |
                                              //aiProcess_PreTransformVertices |
                                              aiProcess_SortByPType |
                                              //aiProcess_FindDegenerates |
                                              //aiProcess_FindInvalidData |
                                              //aiProcess_GenUVCoords |
                                              aiProcess_RemoveRedundantMaterials |
                                              aiProcess_OptimizeGraph);

    if (!pScene)
    {
        QMessageBox::warning(this, "Error", "Error: Couldn't import file!");
        return;
    }

    CModel *pModel = nullptr;
    CMaterialSet *pSet = CMaterialLoader::ImportAssimpMaterials(pScene, EGame::Prime);
    pModel = CModelLoader::ImportAssimpNode(pScene->mRootNode, pScene, *pSet);

    SetActiveModel(pModel);
    SET_WINDOWTITLE_APPVARS("%APP_FULL_NAME% - Model Editor: Untitled");
    mOutputFilename = "";
    gpResourceStore->DestroyUnreferencedResources();
}

void CModelEditorWindow::Save()
{
    if (!mpCurrentModel) return;
    bool SaveSuccess = mpCurrentModel->Entry()->Save();

    if (SaveSuccess)
        gpEdApp->NotifyAssetsModified();
}

void CModelEditorWindow::ConvertToDDS()
{
    QString Input = QFileDialog::getOpenFileName(this, "Retro Texture (*.TXTR)", "", "*.TXTR");
    if (Input.isEmpty()) return;

    TString TexFilename = TO_TSTRING(Input);
    CFileInStream InTextureFile(TexFilename, IOUtil::eLittleEndian);
    CTexture *pTex = CTextureDecoder::LoadTXTR( InTextureFile, nullptr );

    TString OutName = TexFilename.GetFilePathWithoutExtension() + ".dds";
    CFileOutStream Out(OutName, IOUtil::eLittleEndian);
    if (!Out.IsValid()) QMessageBox::warning(this, "Error", "Couldn't open output DDS!");

    else
    {
        bool Success = pTex->WriteDDS(Out);
        if (!Success) QMessageBox::warning(this, "Error", "Couldn't write output DDS!");
        else QMessageBox::information(this, "Success", "Successfully converted to DDS!");
    }

    delete pTex;
}

void CModelEditorWindow::ConvertToTXTR()
{
    QString Input = QFileDialog::getOpenFileName(this, "DirectDraw Surface (*.dds)", "", "*.dds");
    if (Input.isEmpty()) return;

    TString TexFilename = TO_TSTRING(Input);
    CFileInStream InTextureFile = CFileInStream(TexFilename, IOUtil::eLittleEndian);
    CTexture *pTex = CTextureDecoder::LoadDDS(InTextureFile, nullptr);
    TString OutName = TexFilename.GetFilePathWithoutExtension() + ".txtr";

    if ((pTex->TexelFormat() != eDXT1) || (pTex->NumMipMaps() > 1))
        QMessageBox::warning(this, "Error", "Can't convert DDS to TXTR! Save your texture as a DXT1 DDS with no mipmaps, then try again.");

    else
    {
        CFileOutStream Out(OutName, IOUtil::eBigEndian);
        if (!Out.IsValid()) QMessageBox::warning(this, "Error", "Couldn't open output TXTR!");

        else
        {
            CTextureEncoder::EncodeTXTR(Out, pTex, eGX_CMPR);
            QMessageBox::information(this, "Success", "Successfully converted to TXTR!");
        }
    }
}

void CModelEditorWindow::SetMeshPreview()
{
    ui->Viewport->SetDrawMode(CModelEditorViewport::eDrawMesh);
}

void CModelEditorWindow::SetSpherePreview()
{
    ui->Viewport->SetDrawMode(CModelEditorViewport::eDrawSphere);
}

void CModelEditorWindow::SetFlatPreview()
{
    ui->Viewport->SetDrawMode(CModelEditorViewport::eDrawSquare);
}

void CModelEditorWindow::ClearColorChanged(const QColor& rkNewColor)
{
    CColor Color = TO_CCOLOR(rkNewColor);
    ui->Viewport->SetClearColor(Color);
}

void CModelEditorWindow::ToggleCameraMode()
{
    CCamera *pCam = &ui->Viewport->Camera();

    if (pCam->MoveMode() == eOrbitCamera)
        pCam->SetMoveMode(eFreeCamera);

    else if (pCam->MoveMode() == eFreeCamera)
    {
        pCam->SetMoveMode(eOrbitCamera);

        CVector3f Pos = pCam->Position();
        CVector3f Target = mpCurrentModelNode->AABox().Center();
        pCam->SetOrbitDistance(Pos.Distance(Target));
    }
}

void CModelEditorWindow::ToggleGrid(bool Enabled)
{
    ui->Viewport->SetGridEnabled(Enabled);
}

CModelEditorViewport* CModelEditorWindow::Viewport() const
{
    return ui->Viewport;
}
