#include "CLayerEditor.h"
#include "ui_CLayerEditor.h"
#include "../UICommon.h"
#include <Resource/script/CScriptLayer.h>

CLayerEditor::CLayerEditor(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CLayerEditor)
{
    ui->setupUi(this);

    mpArea = nullptr;
    mpModel = new CLayerModel(this);
    ui->LayerSelectComboBox->setModel(mpModel);

    connect(ui->LayerSelectComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(SetCurrentIndex(int)));
    connect(ui->NameLineEdit, SIGNAL(textEdited(QString)), this, SLOT(EditLayerName(QString)));
    connect(ui->ActiveCheckBox, SIGNAL(toggled(bool)), this, SLOT(EditLayerActive(bool)));
}

CLayerEditor::~CLayerEditor()
{
    delete ui;
}

void CLayerEditor::SetArea(CGameArea *pArea)
{
    mpArea = pArea;
    mpModel->SetArea(pArea);
    SetCurrentIndex(0);
}

// ************ SLOTS ************
void CLayerEditor::SetCurrentIndex(int index)
{
    ui->LayerSelectComboBox->blockSignals(true);
    ui->LayerSelectComboBox->setCurrentIndex(index);
    ui->LayerSelectComboBox->blockSignals(false);

    QModelIndex ModelIndex = mpModel->index(index);
    mpCurrentLayer = mpModel->Layer(ModelIndex);

    ui->NameLineEdit->setText(TO_QSTRING(mpCurrentLayer->Name()));
    ui->ActiveCheckBox->setChecked(mpCurrentLayer->IsActive());
}

void CLayerEditor::EditLayerName(const QString &name)
{
    mpCurrentLayer->SetName(name.toStdString());
    ui->LayerSelectComboBox->update();
}

void CLayerEditor::EditLayerActive(bool active)
{
    mpCurrentLayer->SetActive(active);
}
