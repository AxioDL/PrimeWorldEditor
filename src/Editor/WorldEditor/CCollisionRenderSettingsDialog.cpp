#include "CCollisionRenderSettingsDialog.h"
#include "ui_CCollisionRenderSettingsDialog.h"
#include "CWorldEditor.h"
#include "Editor/UICommon.h"

CCollisionRenderSettingsDialog::CCollisionRenderSettingsDialog(CWorldEditor *pEditor, QWidget *pParent /*= 0*/)
    : QDialog(pParent)
    , mpUi(std::make_unique<Ui::CCollisionRenderSettingsDialog>())
    , mpEditor(pEditor)
{
    mpUi->setupUi(this);

    SetupWidgets();
    connect(gpEdApp, &CEditorApplication::ActiveProjectChanged, this, &CCollisionRenderSettingsDialog::SetupWidgets);
    connect(mpUi->HideMaskLineEdit, &QLineEdit::textChanged, this, &CCollisionRenderSettingsDialog::OnHideMaskChanged);
    connect(mpUi->HighlightMaskLineEdit, &QLineEdit::textChanged, this, &CCollisionRenderSettingsDialog::OnHighlightMaskChanged);
    connect(mpUi->WireframeCheckBox, &QCheckBox::toggled, this, &CCollisionRenderSettingsDialog::OnWireframeToggled);
    connect(mpUi->SurfaceTypeCheckBox, &QCheckBox::toggled, this, &CCollisionRenderSettingsDialog::OnSurfaceTypeToggled);
    connect(mpUi->StandableTrisCheckBox, &QCheckBox::toggled, this, &CCollisionRenderSettingsDialog::OnStandableTrisToggled);
    connect(mpUi->AreaBoundsCheckBox, &QCheckBox::toggled, this, &CCollisionRenderSettingsDialog::OnAreaBoundsToggled);
    connect(mpUi->BackfacesCheckBox, &QCheckBox::toggled, this, &CCollisionRenderSettingsDialog::OnBackfacesToggled);

    connect(mpUi->HideShootThruCheckBox, &QCheckBox::toggled, this, &CCollisionRenderSettingsDialog::OnHideCheckboxesToggled);
    connect(mpUi->HideCameraThruCheckBox, &QCheckBox::toggled, this, &CCollisionRenderSettingsDialog::OnHideCheckboxesToggled);
    connect(mpUi->HideScanThruCheckBox, &QCheckBox::toggled, this, &CCollisionRenderSettingsDialog::OnHideCheckboxesToggled);
    connect(mpUi->HideAiWalkThruCheckBox, &QCheckBox::toggled, this, &CCollisionRenderSettingsDialog::OnHideCheckboxesToggled);
    connect(mpUi->HideAiBlockCheckBox, &QCheckBox::toggled, this, &CCollisionRenderSettingsDialog::OnHideCheckboxesToggled);
}

CCollisionRenderSettingsDialog::~CCollisionRenderSettingsDialog() = default;

void CCollisionRenderSettingsDialog::SetupWidgets()
{
    SCollisionRenderSettings& rSettings = mpEditor->Viewport()->CollisionRenderSettings();
    EGame Game = mpEditor->CurrentGame();

    // Set widgets to match current render setting values
    mpUi->HideMaskLineEdit->setText( "0x" + QString::number(rSettings.HideMask, 16).toUpper() );
    mpUi->HighlightMaskLineEdit->setText( "0x" + QString::number(rSettings.HighlightMask, 16).toUpper() );
    mpUi->WireframeCheckBox->setChecked(rSettings.DrawWireframe);
    mpUi->SurfaceTypeCheckBox->setChecked(rSettings.TintWithSurfaceColor);
    mpUi->StandableTrisCheckBox->setChecked(rSettings.TintUnwalkableTris);
    mpUi->AreaBoundsCheckBox->setChecked(rSettings.DrawAreaCollisionBounds);
    mpUi->BackfacesCheckBox->setChecked(rSettings.DrawBackfaces);

    mpUi->HideShootThruCheckBox->setChecked(rSettings.HideMaterial.HasFlag(eCF_ShootThru));
    mpUi->HideCameraThruCheckBox->setChecked(rSettings.HideMaterial.HasFlag(eCF_CameraThru));
    mpUi->HideScanThruCheckBox->setChecked(rSettings.HideMaterial.HasFlag(eCF_ScanThru));
    mpUi->HideAiWalkThruCheckBox->setChecked(rSettings.HideMaterial.HasFlag(eCF_AiWalkThru));
    mpUi->HideAiBlockCheckBox->setChecked(rSettings.HideMaterial.HasFlag(eCF_AiBlock));

    // Toggle visibility of game-exclusive widgets
    mpUi->SurfaceTypeCheckBox->setHidden( Game == EGame::DKCReturns );
    mpUi->StandableTrisCheckBox->setHidden( Game == EGame::DKCReturns );
    mpUi->AreaBoundsCheckBox->setHidden( Game == EGame::DKCReturns );
    mpUi->BackfacesCheckBox->setHidden( Game == EGame::DKCReturns );

    mpUi->VisibilityGroupBox->setHidden( Game == EGame::DKCReturns );
    mpUi->HideShootThruCheckBox->setHidden( Game == EGame::DKCReturns );
    mpUi->HideCameraThruCheckBox->setHidden( Game == EGame::DKCReturns );
    mpUi->HideScanThruCheckBox->setHidden( Game == EGame::DKCReturns );
    mpUi->HideAiWalkThruCheckBox->setHidden( Game == EGame::DKCReturns );
    mpUi->HideAiBlockCheckBox->setHidden( Game < EGame::EchoesDemo || Game == EGame::DKCReturns );
}

void CCollisionRenderSettingsDialog::OnHideMaskChanged(QString NewMask)
{
    TString MaskStr = TO_TSTRING(NewMask);
    uint64 Mask = (MaskStr.IsHexString() ? MaskStr.ToInt64(16) : 0);
    mpEditor->Viewport()->CollisionRenderSettings().HideMask = Mask;
}

void CCollisionRenderSettingsDialog::OnHighlightMaskChanged(QString NewMask)
{
    TString MaskStr = TO_TSTRING(NewMask);
    uint64 Mask = (MaskStr.IsHexString() ? MaskStr.ToInt64(16) : 0);
    mpEditor->Viewport()->CollisionRenderSettings().HighlightMask = Mask;
}

void CCollisionRenderSettingsDialog::OnWireframeToggled(bool Enable)
{
    mpEditor->Viewport()->CollisionRenderSettings().DrawWireframe = Enable;
}

void CCollisionRenderSettingsDialog::OnSurfaceTypeToggled(bool Enable)
{
    mpEditor->Viewport()->CollisionRenderSettings().TintWithSurfaceColor = Enable;
}

void CCollisionRenderSettingsDialog::OnStandableTrisToggled(bool Enable)
{
    mpEditor->Viewport()->CollisionRenderSettings().TintUnwalkableTris = Enable;
}

void CCollisionRenderSettingsDialog::OnAreaBoundsToggled(bool Enable)
{
    mpEditor->Viewport()->CollisionRenderSettings().DrawAreaCollisionBounds = Enable;
}

void CCollisionRenderSettingsDialog::OnBackfacesToggled(bool Enable)
{
    mpEditor->Viewport()->CollisionRenderSettings().DrawBackfaces = Enable;
}

void CCollisionRenderSettingsDialog::OnHideCheckboxesToggled()
{
    CCollisionMaterial& rMat = mpEditor->Viewport()->CollisionRenderSettings().HideMaterial;
    mpUi->HideShootThruCheckBox->isChecked()  ? rMat |= eCF_ShootThru  : rMat &= ~eCF_ShootThru;
    mpUi->HideCameraThruCheckBox->isChecked() ? rMat |= eCF_CameraThru : rMat &= ~eCF_CameraThru;
    mpUi->HideScanThruCheckBox->isChecked()   ? rMat |= eCF_ScanThru   : rMat &= ~eCF_ScanThru;
    mpUi->HideAiWalkThruCheckBox->isChecked() ? rMat |= eCF_AiWalkThru : rMat &= ~eCF_AiWalkThru;
    mpUi->HideAiBlockCheckBox->isChecked()    ? rMat |= eCF_AiBlock    : rMat &= ~eCF_AiBlock;
}
