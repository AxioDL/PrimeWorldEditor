#include "Editor/WorldEditor/CCollisionRenderSettingsDialog.h"
#include "ui_CCollisionRenderSettingsDialog.h"

#include "Editor/UICommon.h"
#include "Editor/WorldEditor/CWorldEditor.h"

#include <QRegularExpression>
#include <QRegularExpressionValidator>

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
    const auto& rSettings = mpEditor->Viewport()->CollisionRenderSettings();
    const auto Game = mpEditor->CurrentGame();

    // Set widgets to match current render setting values
    auto* hexValidator = new QRegularExpressionValidator(QRegularExpression(QStringLiteral("^(?:0[xX])?([0-9]|[A-F]|[a-f]){1,16}$")));
    mpUi->HideMaskLineEdit->setText(QString::number(rSettings.HideMask, 16).toUpper());
    mpUi->HighlightMaskLineEdit->setText(QString::number(rSettings.HighlightMask, 16).toUpper());
    mpUi->HideMaskLineEdit->setValidator(hexValidator);
    mpUi->HighlightMaskLineEdit->setValidator(hexValidator);
    mpUi->HideMaskLineEdit->setToolTip(tr("Must be hex characters (0x optional)"));
    mpUi->HighlightMaskLineEdit->setToolTip(tr("Must be hex characters (0x optional)"));

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
    mpUi->SurfaceTypeCheckBox->setHidden(Game == EGame::DKCReturns);
    mpUi->StandableTrisCheckBox->setHidden(Game == EGame::DKCReturns);
    mpUi->AreaBoundsCheckBox->setHidden(Game == EGame::DKCReturns);
    mpUi->BackfacesCheckBox->setHidden(Game == EGame::DKCReturns);

    mpUi->VisibilityGroupBox->setHidden(Game == EGame::DKCReturns);
    mpUi->HideShootThruCheckBox->setHidden(Game == EGame::DKCReturns);
    mpUi->HideCameraThruCheckBox->setHidden(Game == EGame::DKCReturns);
    mpUi->HideScanThruCheckBox->setHidden(Game == EGame::DKCReturns);
    mpUi->HideAiWalkThruCheckBox->setHidden(Game == EGame::DKCReturns);
    mpUi->HideAiBlockCheckBox->setHidden(Game < EGame::EchoesDemo || Game == EGame::DKCReturns);
}

void CCollisionRenderSettingsDialog::OnHideMaskChanged(const QString& NewMask)
{
    const auto Mask = NewMask.toULongLong(nullptr, 16);
    mpEditor->Viewport()->CollisionRenderSettings().HideMask = Mask;
}

void CCollisionRenderSettingsDialog::OnHighlightMaskChanged(const QString& NewMask)
{
    const auto Mask = NewMask.toULongLong(nullptr, 16);
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
    rMat.AssignFlag(eCF_ShootThru, mpUi->HideShootThruCheckBox->isChecked());
    rMat.AssignFlag(eCF_CameraThru, mpUi->HideCameraThruCheckBox->isChecked());
    rMat.AssignFlag(eCF_ScanThru, mpUi->HideScanThruCheckBox->isChecked());
    rMat.AssignFlag(eCF_AiWalkThru, mpUi->HideAiWalkThruCheckBox->isChecked());
    rMat.AssignFlag(eCF_AiBlock, mpUi->HideAiBlockCheckBox->isChecked());
}
