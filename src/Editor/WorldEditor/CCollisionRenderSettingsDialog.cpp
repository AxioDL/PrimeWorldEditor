#include "CCollisionRenderSettingsDialog.h"
#include "ui_CCollisionRenderSettingsDialog.h"
#include "CWorldEditor.h"
#include "Editor/UICommon.h"

CCollisionRenderSettingsDialog::CCollisionRenderSettingsDialog(CWorldEditor *pEditor, QWidget *pParent /*= 0*/)
    : QDialog(pParent)
    , mpEditor(pEditor)
    , mpUi(new Ui::CCollisionRenderSettingsDialog)
{
    mpUi->setupUi(this);

    SetupWidgets();
    connect(gpEdApp, SIGNAL(ActiveProjectChanged(CGameProject*)), this, SLOT(SetupWidgets()));
    connect(mpUi->HideMaskLineEdit, SIGNAL(textChanged(QString)), this, SLOT(OnHideMaskChanged(QString)));
    connect(mpUi->HighlightMaskLineEdit, SIGNAL(textChanged(QString)), this, SLOT(OnHighlightMaskChanged(QString)));
    connect(mpUi->WireframeCheckBox, SIGNAL(toggled(bool)), this, SLOT(OnWireframeToggled(bool)));
    connect(mpUi->SurfaceTypeCheckBox, SIGNAL(toggled(bool)), this, SLOT(OnSurfaceTypeToggled(bool)));
    connect(mpUi->StandableTrisCheckBox, SIGNAL(toggled(bool)), this, SLOT(OnStandableTrisToggled(bool)));
    connect(mpUi->AreaBoundsCheckBox, SIGNAL(toggled(bool)), this, SLOT(OnAreaBoundsToggled(bool)));
    connect(mpUi->BackfacesCheckBox, SIGNAL(toggled(bool)), this, SLOT(OnBackfacesToggled(bool)));

    connect(mpUi->HideShootThruCheckBox, SIGNAL(toggled(bool)), this, SLOT(OnHideCheckboxesToggled()));
    connect(mpUi->HideCameraThruCheckBox, SIGNAL(toggled(bool)), this, SLOT(OnHideCheckboxesToggled()));
    connect(mpUi->HideScanThruCheckBox, SIGNAL(toggled(bool)), this, SLOT(OnHideCheckboxesToggled()));
    connect(mpUi->HideAiWalkThruCheckBox, SIGNAL(toggled(bool)), this, SLOT(OnHideCheckboxesToggled()));
    connect(mpUi->HideAiBlockCheckBox, SIGNAL(toggled(bool)), this, SLOT(OnHideCheckboxesToggled()));
}

CCollisionRenderSettingsDialog::~CCollisionRenderSettingsDialog()
{
    delete mpUi;
}

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
