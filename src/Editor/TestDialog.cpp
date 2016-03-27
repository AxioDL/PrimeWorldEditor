#include "TestDialog.h"
#include "ui_TestDialog.h"

TestDialog::TestDialog(QWidget *pParent)
    : QDialog(pParent)
    , ui(new Ui::TestDialog)
{
    ui->setupUi(this);
}

TestDialog::~TestDialog()
{
    delete ui;
}
