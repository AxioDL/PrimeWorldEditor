#ifndef CREPACKINFODIALOG_H
#define CREPACKINFODIALOG_H

#include <QDialog>

namespace Ui {
class CRepackInfoDialog;
}

class CRepackInfoDialog : public QDialog
{
    Q_OBJECT
    Ui::CRepackInfoDialog *ui;

public:
    explicit CRepackInfoDialog(QString TargetFolder, QString ListFile, QString OutputPak, QWidget *pParent = 0);
    ~CRepackInfoDialog();

    bool TargetFolderValid() const;
    bool ListFileValid() const;
    bool OutputPakValid() const;

    QString TargetFolder() const;
    QString ListFile() const;
    QString OutputPak() const;

public slots:
    void BrowseFolderClicked();
    void BrowseListClicked();
    void BrowseOutPakClicked();
    void UpdateStatus();
};

#endif // CREPACKINFODIALOG_H
