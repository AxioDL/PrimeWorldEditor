#include "CMaterialEditor.h"
#include "ui_CMaterialEditor.h"

CMaterialEditor::CMaterialEditor(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CMaterialEditor)
{
    ui->setupUi(this);
}

CMaterialEditor::~CMaterialEditor()
{
    delete ui;
}
