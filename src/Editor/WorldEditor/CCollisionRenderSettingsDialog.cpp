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

    connect(mpUi->HideMaskLineEdit, SIGNAL(textChanged(QString)), this, SLOT(OnHideMaskChanged(QString)));
    connect(mpUi->HighlightMaskLineEdit, SIGNAL(textChanged(QString)), this, SLOT(OnHighlightMaskChanged(QString)));
    connect(mpUi->WireframeCheckBox, SIGNAL(toggled(bool)), this, SLOT(OnWireframeToggled(bool)));
}

CCollisionRenderSettingsDialog::~CCollisionRenderSettingsDialog()
{
    delete mpUi;
}

void CCollisionRenderSettingsDialog::OnHideMaskChanged(QString NewMask)
{
    TString MaskStr = TO_TSTRING(NewMask);
    u64 Mask = (MaskStr.IsHexString() ? MaskStr.ToInt64(16) : 0);
    mpEditor->Viewport()->CollisionRenderSettings().HideMask = Mask;
}

void CCollisionRenderSettingsDialog::OnHighlightMaskChanged(QString NewMask)
{
    TString MaskStr = TO_TSTRING(NewMask);
    u64 Mask = (MaskStr.IsHexString() ? MaskStr.ToInt64(16) : 0);
    mpEditor->Viewport()->CollisionRenderSettings().HighlightMask = Mask;
}

void CCollisionRenderSettingsDialog::OnWireframeToggled(bool Enable)
{
    mpEditor->Viewport()->CollisionRenderSettings().DrawWireframe = Enable;
}
