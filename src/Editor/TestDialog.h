#ifndef TESTDIALOG_H
#define TESTDIALOG_H

#include <QDialog>
#include "Editor/PropertyEdit/CPropertyModel.h"

namespace Ui {
class TestDialog;
}

class TestDialog : public QDialog
{
    Q_OBJECT
    CPropertyModel *mpModel;

public:
    explicit TestDialog(QWidget *pParent = 0);
    ~TestDialog();

private:
    Ui::TestDialog *ui;
};

#endif // TESTDIALOG_H
