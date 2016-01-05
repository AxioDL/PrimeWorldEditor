#ifndef CERRORLOGDIALOG_H
#define CERRORLOGDIALOG_H

#include <QDialog>

namespace Ui {
class CErrorLogDialog;
}

class CErrorLogDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CErrorLogDialog(QWidget *parent = 0);
    ~CErrorLogDialog();
    bool GatherErrors();

private:
    Ui::CErrorLogDialog *ui;
};

#endif // CERRORLOGDIALOG_H
