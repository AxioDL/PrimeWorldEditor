#ifndef TESTDIALOG_H
#define TESTDIALOG_H

#include <QDialog>

namespace Ui {
class TestDialog;
}

class TestDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TestDialog(QWidget *pParent = 0);
    ~TestDialog();

public slots:
    void OnSpinBoxChanged(int NewValue);
    void OnFind();

private:
    Ui::TestDialog *ui;
};

#endif // TESTDIALOG_H
