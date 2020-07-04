#include "CModelEditorWindow.h"
#include "ui_CModelEditorWindow.h"
#include "Editor/UICommon.h"
#include "Editor/Widgets/WColorPicker.h"

#include <Common/TString.h>
#include <Core/Render/CDrawUtil.h>
#include <Core/Render/CRenderer.h>
#include <Core/Resource/Cooker/CTextureEncoder.h>
#include <Core/Resource/Factory/CModelLoader.h>
#include <Core/Resource/Factory/CMaterialLoader.h>
#include <Core/Resource/Factory/CTextureDecoder.h>
#include <Core/Scene/CScene.h>

#include <QFileDialog>
#include <QMessageBox>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

CModelEditorWindow::CModelEditorWindow(CModel *pModel, QWidget *pParent)
    : IEditor(pParent)
    , ui(std::make_unique<Ui::CModelEditorWindow>())
    , mpScene(std::make_unique<CScene>())
    , mpCurrentModelNode(std::make_unique<CModelNode>(mpScene.get(), UINT32_MAX))
{
    ui->setupUi(this);
    ui->ActionSave->setEnabled( pModel->Game() == EGame::Prime ); // we don't support saving games later than MP1
    REPLACE_WINDOWTITLE_APPVARS;

    ui->Viewport->SetNode(mpCurrentModelNode.get());
    ui->Viewport->SetClearColor(CColor(0.3f, 0.3f, 0.3f, 1.f));

    CCamera& rCamera = ui->Viewport->Camera();
    rCamera.Snap(CVector3f(0, 3, 1));
    rCamera.SetMoveMode(ECameraMoveMode::Orbit);
    rCamera.SetOrbit(CVector3f(0, 0, 1), 3.f);
    rCamera.SetMoveSpeed(0.5f);

    // UI initialization
    UpdateAnimParamUI(EUVAnimMode::NoUVAnim);
    ui->IndTextureResSelector->SetTypeFilter(pModel->Game(), "TXTR");
    ui->PassTextureResSelector->SetTypeFilter(pModel->Game(), "TXTR");
    ui->PassTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->PassTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->ClearColorPicker->SetColor(QColor(76, 76, 76, 255));

    // Editor UI Signal/Slot setup
    ui->SetSelectionComboBox->setProperty          ("ModelEditorWidgetType", (int) EModelEditorWidget::SetSelectComboBox);
    ui->MatSelectionComboBox->setProperty          ("ModelEditorWidgetType", (int) EModelEditorWidget::MatSelectComboBox);
    ui->EnableTransparencyCheck->setProperty       ("ModelEditorWidgetType", (int) EModelEditorWidget::EnableTransparencyCheckBox);
    ui->EnablePunchthroughCheck->setProperty       ("ModelEditorWidgetType", (int) EModelEditorWidget::EnablePunchthroughCheckBox);
    ui->EnableReflectionCheck->setProperty         ("ModelEditorWidgetType", (int) EModelEditorWidget::EnableReflectionCheckBox);
    ui->EnableSurfaceReflectionCheck->setProperty  ("ModelEditorWidgetType", (int) EModelEditorWidget::EnableSurfaceReflectionCheckBox);
    ui->EnableDepthWriteCheck->setProperty         ("ModelEditorWidgetType", (int) EModelEditorWidget::EnableDepthWriteCheckBox);
    ui->EnableOccluderCheck->setProperty           ("ModelEditorWidgetType", (int) EModelEditorWidget::EnableOccluderCheckBox);
    ui->EnableLightmapCheck->setProperty           ("ModelEditorWidgetType", (int) EModelEditorWidget::EnableLightmapCheckBox);
    ui->EnableDynamicLightingCheck->setProperty    ("ModelEditorWidgetType", (int) EModelEditorWidget::EnableLightingCheckBox);
    ui->SourceBlendComboBox->setProperty           ("ModelEditorWidgetType", (int) EModelEditorWidget::SourceBlendComboBox);
    ui->DestBlendComboBox->setProperty             ("ModelEditorWidgetType", (int) EModelEditorWidget::DestBlendComboBox);
    ui->KonstColorPickerA->setProperty             ("ModelEditorWidgetType", (int) EModelEditorWidget::KonstColorPickerA);
    ui->KonstColorPickerB->setProperty             ("ModelEditorWidgetType", (int) EModelEditorWidget::KonstColorPickerB);
    ui->KonstColorPickerC->setProperty             ("ModelEditorWidgetType", (int) EModelEditorWidget::KonstColorPickerC);
    ui->KonstColorPickerD->setProperty             ("ModelEditorWidgetType", (int) EModelEditorWidget::KonstColorPickerD);
    ui->IndTextureResSelector->setProperty         ("ModelEditorWidgetType", (int) EModelEditorWidget::IndTextureResSelector);
    ui->PassTable->setProperty                     ("ModelEditorWidgetType", (int) EModelEditorWidget::PassTableWidget);
    ui->TevKColorSelComboBox->setProperty          ("ModelEditorWidgetType", (int) EModelEditorWidget::TevKColorSelComboBox);
    ui->TevKAlphaSelComboBox->setProperty          ("ModelEditorWidgetType", (int) EModelEditorWidget::TevKAlphaSelComboBox);
    ui->TevRasSelComboBox->setProperty             ("ModelEditorWidgetType", (int) EModelEditorWidget::TevRasSelComboBox);
    ui->TexCoordSrcComboBox->setProperty           ("ModelEditorWidgetType", (int) EModelEditorWidget::TevTexSourceComboBox);
    ui->PassTextureResSelector->setProperty        ("ModelEditorWidgetType", (int) EModelEditorWidget::PassTextureResSelector);
    ui->TevColor1ComboBox->setProperty             ("ModelEditorWidgetType", (int) EModelEditorWidget::TevColorComboBoxA);
    ui->TevColor2ComboBox->setProperty             ("ModelEditorWidgetType", (int) EModelEditorWidget::TevColorComboBoxB);
    ui->TevColor3ComboBox->setProperty             ("ModelEditorWidgetType", (int) EModelEditorWidget::TevColorComboBoxC);
    ui->TevColor4ComboBox->setProperty             ("ModelEditorWidgetType", (int) EModelEditorWidget::TevColorComboBoxD);
    ui->TevColorOutputComboBox->setProperty        ("ModelEditorWidgetType", (int) EModelEditorWidget::TevColorOutputComboBox);
    ui->TevAlpha1ComboBox->setProperty             ("ModelEditorWidgetType", (int) EModelEditorWidget::TevAlphaComboBoxA);
    ui->TevAlpha2ComboBox->setProperty             ("ModelEditorWidgetType", (int) EModelEditorWidget::TevAlphaComboBoxB);
    ui->TevAlpha3ComboBox->setProperty             ("ModelEditorWidgetType", (int) EModelEditorWidget::TevAlphaComboBoxC);
    ui->TevAlpha4ComboBox->setProperty             ("ModelEditorWidgetType", (int) EModelEditorWidget::TevAlphaComboBoxD);
    ui->TevAlphaOutputComboBox->setProperty        ("ModelEditorWidgetType", (int) EModelEditorWidget::TevAlphaOutputComboBox);
    ui->AnimTypeComboBox->setProperty              ("ModelEditorWidgetType", (int) EModelEditorWidget::AnimModeComboBox);
    ui->AnimParamASpinBox->setProperty             ("ModelEditorWidgetType", (int) EModelEditorWidget::AnimParamASpinBox);
    ui->AnimParamBSpinBox->setProperty             ("ModelEditorWidgetType", (int) EModelEditorWidget::AnimParamBSpinBox);
    ui->AnimParamCSpinBox->setProperty             ("ModelEditorWidgetType", (int) EModelEditorWidget::AnimParamCSpinBox);
    ui->AnimParamDSpinBox->setProperty             ("ModelEditorWidgetType", (int) EModelEditorWidget::AnimParamDSpinBox);

    connect(ui->ActionImport, &QAction::triggered, this, &CModelEditorWindow::Import);
    connect(ui->ActionSave, &QAction::triggered, this, &CModelEditorWindow::Save);
    connect(ui->ActionConvertToDDS, &QAction::triggered, this, &CModelEditorWindow::ConvertToDDS);
    connect(ui->ActionConvertToTXTR, &QAction::triggered, this, &CModelEditorWindow::ConvertToTXTR);
    connect(ui->MeshPreviewButton, &QPushButton::clicked, this, &CModelEditorWindow::SetMeshPreview);
    connect(ui->SpherePreviewButton, &QPushButton::clicked, this, &CModelEditorWindow::SetSpherePreview);
    connect(ui->FlatPreviewButton, &QPushButton::clicked, this, &CModelEditorWindow::SetFlatPreview);
    connect(ui->ClearColorPicker, &WColorPicker::ColorChanged, this, &CModelEditorWindow::ClearColorChanged);
    connect(ui->CameraModeButton, &QPushButton::clicked, this, &CModelEditorWindow::ToggleCameraMode);
    connect(ui->ToggleGridButton, &QPushButton::toggled, this, &CModelEditorWindow::ToggleGrid);

    connect(ui->SetSelectionComboBox, qOverload<int>(&QComboBox::currentIndexChanged), this, qOverload<int>(&CModelEditorWindow::UpdateUI));
    connect(ui->MatSelectionComboBox, qOverload<int>(&QComboBox::currentIndexChanged), this, qOverload<int>(&CModelEditorWindow::UpdateUI));
    connect(ui->EnableTransparencyCheck, &QCheckBox::toggled, this, qOverload<int>(&CModelEditorWindow::UpdateMaterial));
    connect(ui->EnablePunchthroughCheck, &QCheckBox::toggled, this, qOverload<int>(&CModelEditorWindow::UpdateMaterial));
    connect(ui->EnableReflectionCheck, &QCheckBox::toggled, this, qOverload<int>(&CModelEditorWindow::UpdateMaterial));
    connect(ui->EnableSurfaceReflectionCheck, &QCheckBox::toggled, this, qOverload<int>(&CModelEditorWindow::UpdateMaterial));
    connect(ui->EnableDepthWriteCheck, &QCheckBox::toggled, this, qOverload<int>(&CModelEditorWindow::UpdateMaterial));
    connect(ui->EnableOccluderCheck, &QCheckBox::toggled, this, qOverload<int>(&CModelEditorWindow::UpdateMaterial));
    connect(ui->EnableLightmapCheck, &QCheckBox::toggled, this, qOverload<int>(&CModelEditorWindow::UpdateMaterial));
    connect(ui->EnableDynamicLightingCheck, &QCheckBox::toggled, this, qOverload<int>(&CModelEditorWindow::UpdateMaterial));
    connect(ui->SourceBlendComboBox, qOverload<int>(&QComboBox::currentIndexChanged), this, qOverload<int>(&CModelEditorWindow::UpdateMaterial));
    connect(ui->DestBlendComboBox, qOverload<int>(&QComboBox::currentIndexChanged), this, qOverload<int>(&CModelEditorWindow::UpdateMaterial));
    connect(ui->KonstColorPickerA, &WColorPicker::ColorChanged, this, qOverload<QColor>(&CModelEditorWindow::UpdateMaterial));
    connect(ui->KonstColorPickerB, &WColorPicker::ColorChanged, this, qOverload<QColor>(&CModelEditorWindow::UpdateMaterial));
    connect(ui->KonstColorPickerC, &WColorPicker::ColorChanged, this, qOverload<QColor>(&CModelEditorWindow::UpdateMaterial));
    connect(ui->KonstColorPickerD, &WColorPicker::ColorChanged, this, qOverload<QColor>(&CModelEditorWindow::UpdateMaterial));
    connect(ui->PassTable, &QTableWidget::cellClicked, this, qOverload<int, int>(&CModelEditorWindow::UpdateMaterial));
    connect(ui->TevKColorSelComboBox, qOverload<int>(&QComboBox::currentIndexChanged), this, qOverload<int>(&CModelEditorWindow::UpdateMaterial));
    connect(ui->TevKAlphaSelComboBox, qOverload<int>(&QComboBox::currentIndexChanged), this, qOverload<int>(&CModelEditorWindow::UpdateMaterial));
    connect(ui->TevRasSelComboBox, qOverload<int>(&QComboBox::currentIndexChanged), this, qOverload<int>(&CModelEditorWindow::UpdateMaterial));
    connect(ui->TexCoordSrcComboBox, qOverload<int>(&QComboBox::currentIndexChanged), this, qOverload<int>(&CModelEditorWindow::UpdateMaterial));
    connect(ui->PassTextureResSelector, SIGNAL(ResourceChanged(QString)), this, SLOT(UpdateMaterial(QString)));
    connect(ui->TevColor1ComboBox, qOverload<int>(&QComboBox::currentIndexChanged), this, qOverload<int>(&CModelEditorWindow::UpdateMaterial));
    connect(ui->TevColor2ComboBox, qOverload<int>(&QComboBox::currentIndexChanged), this, qOverload<int>(&CModelEditorWindow::UpdateMaterial));
    connect(ui->TevColor3ComboBox, qOverload<int>(&QComboBox::currentIndexChanged), this, qOverload<int>(&CModelEditorWindow::UpdateMaterial));
    connect(ui->TevColor4ComboBox, qOverload<int>(&QComboBox::currentIndexChanged), this, qOverload<int>(&CModelEditorWindow::UpdateMaterial));
    connect(ui->TevColorOutputComboBox, qOverload<int>(&QComboBox::currentIndexChanged), this, qOverload<int>(&CModelEditorWindow::UpdateMaterial));
    connect(ui->TevAlpha1ComboBox, qOverload<int>(&QComboBox::currentIndexChanged), this, qOverload<int>(&CModelEditorWindow::UpdateMaterial));
    connect(ui->TevAlpha2ComboBox, qOverload<int>(&QComboBox::currentIndexChanged), this, qOverload<int>(&CModelEditorWindow::UpdateMaterial));
    connect(ui->TevAlpha3ComboBox, qOverload<int>(&QComboBox::currentIndexChanged), this, qOverload<int>(&CModelEditorWindow::UpdateMaterial));
    connect(ui->TevAlpha4ComboBox, qOverload<int>(&QComboBox::currentIndexChanged), this, qOverload<int>(&CModelEditorWindow::UpdateMaterial));
    connect(ui->TevAlphaOutputComboBox, qOverload<int>(&QComboBox::currentIndexChanged), this, qOverload<int>(&CModelEditorWindow::UpdateMaterial));
    connect(ui->AnimTypeComboBox, qOverload<int>(&QComboBox::currentIndexChanged), this, qOverload<int>(&CModelEditorWindow::UpdateMaterial));
    connect(ui->AnimParamASpinBox, qOverload<double>(&WDraggableSpinBox::valueChanged), this, qOverload<double>(&CModelEditorWindow::UpdateMaterial));
    connect(ui->AnimParamBSpinBox, qOverload<double>(&WDraggableSpinBox::valueChanged), this, qOverload<double>(&CModelEditorWindow::UpdateMaterial));
    connect(ui->AnimParamCSpinBox, qOverload<double>(&WDraggableSpinBox::valueChanged), this, qOverload<double>(&CModelEditorWindow::UpdateMaterial));
    connect(ui->AnimParamDSpinBox, qOverload<double>(&WDraggableSpinBox::valueChanged), this, qOverload<double>(&CModelEditorWindow::UpdateMaterial));
    // That was fun

    SetActiveModel(pModel);
}

CModelEditorWindow::~CModelEditorWindow() = default;

bool CModelEditorWindow::Save()
{
    if (!mpCurrentModel) return true;
    bool SaveSuccess = mpCurrentModel->Entry()->Save();

    if (SaveSuccess)
        gpEdApp->NotifyAssetsModified();

    return SaveSuccess;
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

    const uint32 NumVertices = (pModel ? pModel->GetVertexCount() : 0);
    const uint32 NumTriangles = (pModel ? pModel->GetTriangleCount() : 0);
    const uint32 NumMats = (pModel ? pModel->GetMatCount() : 0);
    const uint32 NumMatSets = (pModel ? pModel->GetMatSetCount() : 0);
    ui->MeshInfoLabel->setText(tr("%1 vertices, %2 triangles").arg(NumVertices).arg(NumTriangles));
    ui->MatInfoLabel->setText(tr("%1 materials, %2 set%3").arg(NumMats).arg(NumMatSets).arg(NumMatSets == 1 ? QString{} : tr("s")));

    // Set items in matset combo box
    ui->SetSelectionComboBox->blockSignals(true);
    ui->SetSelectionComboBox->clear();

    for (uint32 iSet = 0; iSet < NumMatSets; iSet++)
        ui->SetSelectionComboBox->addItem(tr("Set #%1").arg(iSet + 1));

    ui->SetSelectionComboBox->setCurrentIndex(0);
    ui->SetSelectionComboBox->blockSignals(false);

    // Set items in mat combo box
    ui->MatSelectionComboBox->blockSignals(true);
    ui->MatSelectionComboBox->clear();

    for (size_t iMat = 0; iMat < NumMats; iMat++)
    {
        const TString MatName = pModel->GetMaterialByIndex(0, iMat)->Name();
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
    if (!mpCurrentModel)
        return;

    const auto SetIndex = static_cast<uint32>(ui->SetSelectionComboBox->currentIndex());
    mpCurrentMat = mpCurrentModel->GetMaterialByIndex(SetIndex, MatIndex);
    ui->Viewport->SetActiveMaterial(mpCurrentMat);
    if (!mpCurrentMat)
        return;

    // Set up UI
    FMaterialOptions Settings = mpCurrentMat->Options();

    mIgnoreSignals = true;
    ui->EnableTransparencyCheck->setChecked( Settings & EMaterialOption::Transparent );
    ui->EnablePunchthroughCheck->setChecked( Settings & EMaterialOption::Masked );
    ui->EnableReflectionCheck->setChecked( Settings & EMaterialOption::Reflection );
    ui->EnableSurfaceReflectionCheck->setChecked( Settings & EMaterialOption::SurfaceReflection );
    ui->EnableDepthWriteCheck->setChecked( Settings & EMaterialOption::DepthWrite );
    ui->EnableOccluderCheck->setChecked( Settings & EMaterialOption::Occluder );
    ui->EnableLightmapCheck->setChecked( Settings & EMaterialOption::Lightmap );
    ui->EnableDynamicLightingCheck->setChecked( mpCurrentMat->IsLightingEnabled() );

    uint32 SrcFac = (uint32) mpCurrentMat->BlendSrcFac();
    uint32 DstFac = (uint32) mpCurrentMat->BlendDstFac();
    if (SrcFac >= 0x300) SrcFac -= 0x2FE;
    if (DstFac >= 0x300) DstFac -= 0x2FE;
    ui->SourceBlendComboBox->setCurrentIndex(SrcFac);
    ui->DestBlendComboBox->setCurrentIndex(DstFac);

    if (Settings & EMaterialOption::IndStage)
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

        auto *pItemA = new QTableWidgetItem(tr("Pass #%1: %2").arg(iPass + 1).arg(TO_QSTRING(pPass->NamedType())));
        auto *pItemB = new QTableWidgetItem();

        if (pPass->IsEnabled())
            pItemB->setIcon(QIcon(QStringLiteral(":/icons/Show.svg")));
        else
            pItemB->setIcon(QIcon(QStringLiteral(":/icons/Hide.svg")));

        ui->PassTable->setItem(iPass, 0, pItemA);
        ui->PassTable->setItem(iPass, 1, pItemB);
    }

    // Set up the tex coord source combo box so it only shows vertex attributes that exist on this material
    ui->TexCoordSrcComboBox->clear();
    FVertexDescription Desc = mpCurrentMat->VtxDesc();

    ui->TexCoordSrcComboBox->addItem(tr("None"));
    if ((Desc & EVertexAttribute::Position) != 0) ui->TexCoordSrcComboBox->addItem(tr("Position"));
    if ((Desc & EVertexAttribute::Normal) != 0)   ui->TexCoordSrcComboBox->addItem(tr("Normal"));
    if ((Desc & EVertexAttribute::Tex0) != 0)     ui->TexCoordSrcComboBox->addItem(tr("Tex Coord 1"));
    if ((Desc & EVertexAttribute::Tex1) != 0)     ui->TexCoordSrcComboBox->addItem(tr("Tex Coord 2"));
    if ((Desc & EVertexAttribute::Tex2) != 0)     ui->TexCoordSrcComboBox->addItem(tr("Tex Coord 3"));
    if ((Desc & EVertexAttribute::Tex3) != 0)     ui->TexCoordSrcComboBox->addItem(tr("Tex Coord 4"));
    if ((Desc & EVertexAttribute::Tex4) != 0)     ui->TexCoordSrcComboBox->addItem(tr("Tex Coord 5"));
    if ((Desc & EVertexAttribute::Tex5) != 0)     ui->TexCoordSrcComboBox->addItem(tr("Tex Coord 6"));
    if ((Desc & EVertexAttribute::Tex6) != 0)     ui->TexCoordSrcComboBox->addItem(tr("Tex Coord 7"));
    if ((Desc & EVertexAttribute::Tex7) != 0)     ui->TexCoordSrcComboBox->addItem(tr("Tex Coord 8"));

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

    EUVAnimMode AnimMode = mpCurrentPass->AnimMode();
    ui->AnimTypeComboBox->setCurrentIndex((int) AnimMode + 1);
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
    case EModelEditorWidget::AddPassButton:
        break;

    case EModelEditorWidget::DeletePassButton:
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

    case EModelEditorWidget::SourceBlendComboBox:
    case EModelEditorWidget::DestBlendComboBox:
    {
        GLenum SourceFac = ui->SourceBlendComboBox->currentIndex();
        GLenum DstFac = ui->DestBlendComboBox->currentIndex();
        if (SourceFac > 1) SourceFac += 0x2FE;
        if (DstFac > 1) DstFac += 0x2FE;
        mpCurrentMat->SetBlendMode(SourceFac, DstFac);
        break;
    }

    case EModelEditorWidget::TevKColorSelComboBox:
        if (Value >= 8) Value += 4;
        mpCurrentPass->SetKColorSel((ETevKSel) Value);
        break;

    case EModelEditorWidget::TevKAlphaSelComboBox:
        if (Value >= 8) Value += 8;
        mpCurrentPass->SetKAlphaSel((ETevKSel) Value);
        break;

    case EModelEditorWidget::TevRasSelComboBox:
        if (Value == 7) Value = kRasColorNull;
        mpCurrentPass->SetRasSel((ETevRasSel) Value);
        break;

    case EModelEditorWidget::TevTexSelComboBox:
        // todo
        break;

    case EModelEditorWidget::TevTexSourceComboBox:
        if (Value >= 3) Value ++;
        else Value--;
        mpCurrentPass->SetTexCoordSource(Value);
        break;

    case EModelEditorWidget::TevColorComboBoxA:
    case EModelEditorWidget::TevColorComboBoxB:
    case EModelEditorWidget::TevColorComboBoxC:
    case EModelEditorWidget::TevColorComboBoxD:
    {
        ETevColorInput A = (ETevColorInput) ui->TevColor1ComboBox->currentIndex();
        ETevColorInput B = (ETevColorInput) ui->TevColor2ComboBox->currentIndex();
        ETevColorInput C = (ETevColorInput) ui->TevColor3ComboBox->currentIndex();
        ETevColorInput D = (ETevColorInput) ui->TevColor4ComboBox->currentIndex();
        mpCurrentPass->SetColorInputs(A, B, C, D);
        break;
    }

    case EModelEditorWidget::TevColorOutputComboBox:
        mpCurrentPass->SetColorOutput((ETevOutput) Value);
        break;

    case EModelEditorWidget::TevAlphaComboBoxA:
    case EModelEditorWidget::TevAlphaComboBoxB:
    case EModelEditorWidget::TevAlphaComboBoxC:
    case EModelEditorWidget::TevAlphaComboBoxD:
    {
        ETevAlphaInput A = (ETevAlphaInput) ui->TevAlpha1ComboBox->currentIndex();
        ETevAlphaInput B = (ETevAlphaInput) ui->TevAlpha2ComboBox->currentIndex();
        ETevAlphaInput C = (ETevAlphaInput) ui->TevAlpha3ComboBox->currentIndex();
        ETevAlphaInput D = (ETevAlphaInput) ui->TevAlpha4ComboBox->currentIndex();
        mpCurrentPass->SetAlphaInputs(A, B, C, D);
        break;
    }

    case EModelEditorWidget::TevAlphaOutputComboBox:
        mpCurrentPass->SetAlphaOutput((ETevOutput) Value);
        break;

    case EModelEditorWidget::AnimModeComboBox:
        mpCurrentPass->SetAnimMode((EUVAnimMode) (Value - 1));
        UpdateAnimParamUI((EUVAnimMode) (Value - 1));
        break;
    default: break;
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
            ui->PassTable->item(ValueA, ValueB)->setIcon(QIcon(QStringLiteral(":/icons/Show.svg")));
        else
            ui->PassTable->item(ValueA, ValueB)->setIcon(QIcon(QStringLiteral(":/icons/Hide.svg")));
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
    case EModelEditorWidget::AnimParamASpinBox:
        mpCurrentPass->SetAnimParam(0, (float) Value);
        break;
    case EModelEditorWidget::AnimParamBSpinBox:
        mpCurrentPass->SetAnimParam(1, (float) Value);
        break;
    case EModelEditorWidget::AnimParamCSpinBox:
        mpCurrentPass->SetAnimParam(2, (float) Value);
        break;
    case EModelEditorWidget::AnimParamDSpinBox:
        mpCurrentPass->SetAnimParam(3, (float) Value);
        break;
    default: break;
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

    case EModelEditorWidget::EnableTransparencyCheckBox:
    case EModelEditorWidget::EnablePunchthroughCheckBox:
    case EModelEditorWidget::EnableReflectionCheckBox:
    case EModelEditorWidget::EnableSurfaceReflectionCheckBox:
    case EModelEditorWidget::EnableDepthWriteCheckBox:
    case EModelEditorWidget::EnableOccluderCheckBox:
    case EModelEditorWidget::EnableLightmapCheckBox:
    {
        FMaterialOptions Options = (mpCurrentMat->Options() & 0x2408);
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

    case EModelEditorWidget::EnableLightingCheckBox:
        mpCurrentMat->SetLightingEnabled(Value);
        break;
    default: break;
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
    case EModelEditorWidget::KonstColorPickerA:
        mpCurrentMat->SetKonst(KColor, 0);
        break;
    case EModelEditorWidget::KonstColorPickerB:
        mpCurrentMat->SetKonst(KColor, 1);
        break;
    case EModelEditorWidget::KonstColorPickerC:
        mpCurrentMat->SetKonst(KColor, 2);
        break;
    case EModelEditorWidget::KonstColorPickerD:
        mpCurrentMat->SetKonst(KColor, 3);
        break;
    default: break;
    }
}

void CModelEditorWindow::UpdateMaterial(QString Value)
{
    // This function takes input from WResourceSelectors
    if (!mpCurrentMat) return;
    if (mIgnoreSignals) return;

    EModelEditorWidget Widget = (EModelEditorWidget) sender()->property("ModelEditorWidgetType").toInt();
    TResPtr<CTexture> pTex = gpResourceStore->LoadResource(TO_TSTRING(Value));
    if (pTex->Type() != EResourceType::Texture) pTex = nullptr;

    switch (Widget)
    {
    case EModelEditorWidget::PassTextureResSelector:
        mpCurrentPass->SetTexture(pTex);
        break;

    case EModelEditorWidget::IndTextureResSelector:
        mpCurrentMat->SetIndTexture(pTex);
        break;
    default: break;
    }
}
void CModelEditorWindow::UpdateUI(int Value)
{
    EModelEditorWidget Widget = (EModelEditorWidget) sender()->property("ModelEditorWidgetType").toInt();

    switch (Widget)
    {

    case EModelEditorWidget::SetSelectComboBox:
        mpCurrentModelNode->SetMatSet(Value);
        SetActiveMaterial(ui->MatSelectionComboBox->currentIndex());
        break;

    case EModelEditorWidget::MatSelectComboBox:
        SetActiveMaterial(Value);
        ActivateMatEditUI(true);
        break;
    default: break;
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

void CModelEditorWindow::UpdateAnimParamUI(EUVAnimMode Mode)
{
    // Update the param labels with actual names + hide unused params for each mode.

    switch ((int) Mode)
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
        ui->AnimParamALabel->setText(tr("<b>Horizontal Offset:</b>"));
        ui->AnimParamBLabel->setText(tr("<b>Vertical Offset:</b>"));
        ui->AnimParamCLabel->setText(tr("<b>Horizontal Scale:</b>"));
        ui->AnimParamDLabel->setText(tr("<b>Vertical Scale:</b>"));
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
        ui->AnimParamALabel->setText(tr("<b>Offset:</b>"));
        ui->AnimParamBLabel->setText(tr("<b>Scale:</b>"));
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
        ui->AnimParamALabel->setText(tr("<b>Scale:</b>"));
        ui->AnimParamBLabel->setText(tr("<b>Num Frames:</b>"));
        ui->AnimParamCLabel->setText(tr("<b>Step:</b>"));
        ui->AnimParamDLabel->setText(tr("<b>Time Offset:</b>"));
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
        ui->AnimParamALabel->setText(tr("<b>ParamA:</b>"));
        ui->AnimParamBLabel->setText(tr("<b>ParamB:</b>"));
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
    const QString FileName = QFileDialog::getOpenFileName(this, tr("Model"), {}, QStringLiteral("*.obj;*.fbx;*.dae;*.3ds;*.blend"));
    if (FileName.isEmpty())
        return;

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
        QMessageBox::warning(this, tr("Error"), tr("Error: Couldn't import file!"));
        return;
    }

    CModel *pModel = nullptr;
    CMaterialSet *pSet = CMaterialLoader::ImportAssimpMaterials(pScene, EGame::Prime);
    pModel = CModelLoader::ImportAssimpNode(pScene->mRootNode, pScene, *pSet);

    SetActiveModel(pModel);
    SET_WINDOWTITLE_APPVARS(tr("%APP_FULL_NAME% - Model Editor: Untitled"));
    mOutputFilename = "";
    gpResourceStore->DestroyUnreferencedResources();
}

void CModelEditorWindow::ConvertToDDS()
{
    QString Input = QFileDialog::getOpenFileName(this, tr("Retro Texture (*.TXTR)"), {}, QStringLiteral("*.TXTR"));
    if (Input.isEmpty()) return;

    TString TexFilename = TO_TSTRING(Input);
    CFileInStream InTextureFile(TexFilename, EEndian::LittleEndian);
    auto pTex = CTextureDecoder::LoadTXTR( InTextureFile, nullptr );

    const TString OutName = TexFilename.GetFilePathWithoutExtension() + ".dds";
    CFileOutStream Out(OutName, EEndian::LittleEndian);
    if (!Out.IsValid())
    {
        QMessageBox::warning(this, tr("Error"), tr("Couldn't open output DDS!"));
    }
    else
    {
        const bool Success = pTex->WriteDDS(Out);
        if (!Success)
            QMessageBox::warning(this, tr("Error"), tr("Couldn't write output DDS!"));
        else
            QMessageBox::information(this, tr("Success"), tr("Successfully converted to DDS!"));
    }
}

void CModelEditorWindow::ConvertToTXTR()
{
    QString Input = QFileDialog::getOpenFileName(this, "DirectDraw Surface (*.dds)", "", "*.dds");
    if (Input.isEmpty()) return;

    TString TexFilename = TO_TSTRING(Input);
    CFileInStream InTextureFile = CFileInStream(TexFilename, EEndian::LittleEndian);
    auto pTex = CTextureDecoder::LoadDDS(InTextureFile, nullptr);
    TString OutName = TexFilename.GetFilePathWithoutExtension() + ".txtr";

    if ((pTex->TexelFormat() != ETexelFormat::DXT1) || (pTex->NumMipMaps() > 1))
    {
        QMessageBox::warning(this, tr("Error"), tr("Can't convert DDS to TXTR! Save your texture as a DXT1 DDS with no mipmaps, then try again."));
    }
    else
    {
        CFileOutStream Out(OutName, EEndian::BigEndian);
        if (!Out.IsValid())
        {
            QMessageBox::warning(this, tr("Error"), tr("Couldn't open output TXTR!"));
        }
        else
        {
            CTextureEncoder::EncodeTXTR(Out, pTex.get(), ETexelFormat::GX_CMPR);
            QMessageBox::information(this, tr("Success"), tr("Successfully converted to TXTR!"));
        }
    }
}

void CModelEditorWindow::SetMeshPreview()
{
    ui->Viewport->SetDrawMode(CModelEditorViewport::EDrawMode::DrawMesh);
}

void CModelEditorWindow::SetSpherePreview()
{
    ui->Viewport->SetDrawMode(CModelEditorViewport::EDrawMode::DrawSphere);
}

void CModelEditorWindow::SetFlatPreview()
{
    ui->Viewport->SetDrawMode(CModelEditorViewport::EDrawMode::DrawSquare);
}

void CModelEditorWindow::ClearColorChanged(const QColor& rkNewColor)
{
    CColor Color = TO_CCOLOR(rkNewColor);
    ui->Viewport->SetClearColor(Color);
}

void CModelEditorWindow::ToggleCameraMode()
{
    CCamera *pCam = &ui->Viewport->Camera();

    if (pCam->MoveMode() == ECameraMoveMode::Orbit)
        pCam->SetMoveMode(ECameraMoveMode::Free);

    else if (pCam->MoveMode() == ECameraMoveMode::Free)
    {
        pCam->SetMoveMode(ECameraMoveMode::Orbit);

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
