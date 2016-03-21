#include "CRepackInfoDialog.h"
#include "ui_CRepackInfoDialog.h"
#include "Editor/CPakToolDialog.h"
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QPushButton>

CRepackInfoDialog::CRepackInfoDialog(QString TargetFolder, QString ListFile, QString OutputPak, QWidget *pParent /*= 0*/)
    : QDialog(pParent)
    , ui(new Ui::CRepackInfoDialog)
{
    ui->setupUi(this);

    ui->FolderLineEdit->setText(TargetFolder);
    ui->FileListLineEdit->setText(ListFile);
    ui->OutputPakLineEdit->setText(OutputPak);
    UpdateStatus();

    connect(ui->FolderToolButton, SIGNAL(clicked()), this, SLOT(BrowseFolderClicked()));
    connect(ui->FileListToolButton, SIGNAL(clicked()), this, SLOT(BrowseListClicked()));
    connect(ui->OutputPakToolButton, SIGNAL(clicked()), this, SLOT(BrowseOutPakClicked()));
    connect(ui->FolderLineEdit, SIGNAL(textChanged(QString)), this, SLOT(UpdateStatus()));
    connect(ui->FileListLineEdit, SIGNAL(textChanged(QString)), this, SLOT(UpdateStatus()));
    connect(ui->OutputPakLineEdit, SIGNAL(textChanged(QString)), this, SLOT(UpdateStatus()));
}

CRepackInfoDialog::~CRepackInfoDialog()
{
    delete ui;
}

bool CRepackInfoDialog::TargetFolderValid() const
{
    return QDir(ui->FolderLineEdit->text()).exists();
}

bool CRepackInfoDialog::ListFileValid() const
{
    return QFile::exists(ui->FileListLineEdit->text());
}

bool CRepackInfoDialog::OutputPakValid() const
{
    return QFile::exists(ui->OutputPakLineEdit->text());
}

QString CRepackInfoDialog::TargetFolder() const
{
    return ui->FolderLineEdit->text();
}

QString CRepackInfoDialog::ListFile() const
{
    return ui->FileListLineEdit->text();
}

QString CRepackInfoDialog::OutputPak() const
{
    return ui->OutputPakLineEdit->text();
}

// ************ PUBLIC SLOTS ************
void CRepackInfoDialog::BrowseFolderClicked()
{
    QString Folder = QFileDialog::getExistingDirectory(this, "Choose directory");

    if (!Folder.isEmpty())
    {
        ui->FolderLineEdit->setText(Folder);
        ui->FolderLineEdit->setFocus();
    }
}

void CRepackInfoDialog::BrowseListClicked()
{
    QString List = QFileDialog::getOpenFileName(this, "Open list file", "", "All supported files (*.txt *.pak);;Text file (*.txt);;Pak file (*.pak)");

    if (!List.isEmpty())
    {
        if (List.endsWith(".txt"))
            ui->FileListLineEdit->setText(List);

        else if (List.endsWith(".pak"))
        {
            QString Txt;
            CPakToolDialog::DumpList(List, &Txt);
            ui->FileListLineEdit->setText(Txt);
        }

        ui->FileListLineEdit->setFocus();
    }
}

void CRepackInfoDialog::BrowseOutPakClicked()
{
    QString Pak = QFileDialog::getSaveFileName(this, "Save pak", "", "Pak File (*.pak)");

    if (!Pak.isEmpty())
    {
        ui->OutputPakLineEdit->setText(Pak);
        ui->OutputPakLineEdit->setFocus();
    }
}

void CRepackInfoDialog::UpdateStatus()
{
    static const QString skInvalidStylesheet = "border: 1px solid red";

    ui->FolderLineEdit->setStyleSheet(TargetFolderValid() ? "" : skInvalidStylesheet);
    ui->FileListLineEdit->setStyleSheet(ListFileValid() ? "" : skInvalidStylesheet);
    ui->OutputPakLineEdit->setStyleSheet(OutputPakValid() ? "" : skInvalidStylesheet);
    ui->ButtonBox->button(QDialogButtonBox::Ok)->setEnabled(TargetFolderValid() && ListFileValid() && OutputPakValid());
}
