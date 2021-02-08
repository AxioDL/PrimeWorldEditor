#ifndef CERRORLOGDIALOG_H
#define CERRORLOGDIALOG_H

#include <QDialog>

#include <memory>

namespace Ui {
class CErrorLogDialog;
}

class CErrorLogDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CErrorLogDialog(QWidget *pParent = nullptr);
    ~CErrorLogDialog() override;

    bool GatherErrors();

private:
    std::unique_ptr<Ui::CErrorLogDialog> ui;
};

#endif // CERRORLOGDIALOG_H
