#include "CLayerEditor.h"
#include "ui_CLayerEditor.h"
#include "Editor/UICommon.h"

#include <Core/Resource/Script/CScriptLayer.h>

CLayerEditor::CLayerEditor(QWidget *parent)
    : QDialog(parent)
    , mpModel(new CLayerModel(this))
    , ui(std::make_unique<Ui::CLayerEditor>())
{
    ui->setupUi(this);
    ui->LayerSelectComboBox->setModel(mpModel);

    connect(ui->LayerSelectComboBox, qOverload<int>(&QComboBox::currentIndexChanged), this, &CLayerEditor::SetCurrentIndex);
    connect(ui->NameLineEdit, &QLineEdit::textEdited, this, &CLayerEditor::EditLayerName);
    connect(ui->ActiveCheckBox, &QCheckBox::toggled, this, &CLayerEditor::EditLayerActive);
}

CLayerEditor::~CLayerEditor() = default;

void CLayerEditor::SetArea(CGameArea *pArea)
{
    mpArea = pArea;
    mpModel->SetArea(pArea);
    SetCurrentIndex(0);
}

// ************ SLOTS ************
void CLayerEditor::SetCurrentIndex(int Index)
{
    ui->LayerSelectComboBox->blockSignals(true);
    ui->LayerSelectComboBox->setCurrentIndex(Index);
    ui->LayerSelectComboBox->blockSignals(false);

    QModelIndex ModelIndex = mpModel->index(Index);
    mpCurrentLayer = mpModel->Layer(ModelIndex);

    ui->NameLineEdit->setText(TO_QSTRING(mpCurrentLayer->Name()));
    ui->ActiveCheckBox->setChecked(mpCurrentLayer->IsActive());
}

void CLayerEditor::EditLayerName(const QString& rkName)
{
    mpCurrentLayer->SetName(TO_TSTRING(rkName));
    ui->LayerSelectComboBox->update();
}

void CLayerEditor::EditLayerActive(bool Active)
{
    mpCurrentLayer->SetActive(Active);
}
