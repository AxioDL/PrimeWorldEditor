#include "CAboutDialog.h"
#include "ui_CAboutDialog.h"
#include "UICommon.h"

CAboutDialog::CAboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(std::make_unique<Ui::CAboutDialog>())
{
    ui->setupUi(this);

    QString LabelText = ui->AboutLabel->text();
    REPLACE_APPVARS(LabelText);
    ui->AboutLabel->setText(LabelText);
}

CAboutDialog::~CAboutDialog() = default;
