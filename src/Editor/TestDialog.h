#ifndef TESTDIALOG_H
#define TESTDIALOG_H

#include <QDialog>

#include <memory>

namespace Ui {
class TestDialog;
}

class TestDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TestDialog(QWidget *pParent = nullptr);
    ~TestDialog() override;

public slots:
    void OnSpinBoxChanged(int NewValue);
    void OnFind();

private:
    std::unique_ptr<Ui::TestDialog> ui;
};

#endif // TESTDIALOG_H
