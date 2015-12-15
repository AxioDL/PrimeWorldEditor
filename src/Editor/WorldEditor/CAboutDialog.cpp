#include "CAboutDialog.h"
#include "ui_CAboutDialog.h"

CAboutDialog::CAboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CAboutDialog)
{
    ui->setupUi(this);
}

CAboutDialog::~CAboutDialog()
{
    delete ui;
}
