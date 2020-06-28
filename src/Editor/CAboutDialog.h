#ifndef CABOUTDIALOG_H
#define CABOUTDIALOG_H

#include <QDialog>

#include <memory>

namespace Ui {
class CAboutDialog;
}

class CAboutDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CAboutDialog(QWidget *parent = nullptr);
    ~CAboutDialog() override;

private:
    std::unique_ptr<Ui::CAboutDialog> ui;
};

#endif // CABOUTDIALOG_H
