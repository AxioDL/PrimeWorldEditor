#ifndef CCONFIRMUNLINKDIALOG_H
#define CCONFIRMUNLINKDIALOG_H

#include <QDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

// This class is basically a workaround for the fact that QMessageBox doesn't allow directly controlling button placement
class CConfirmUnlinkDialog : public QDialog
{
    Q_OBJECT
public:
    enum class EChoice
    {
        All,
        IncomingOnly,
        OutgoingOnly,
        Cancel
    };

private:
    QLabel *mpLabel;
    QPushButton *mpAllButton;
    QPushButton *mpIncomingButton;
    QPushButton *mpOutgoingButton;
    QPushButton *mpCancelButton;
    QHBoxLayout *mpButtonLayout;
    QVBoxLayout *mpDialogLayout;

    EChoice mChoice{EChoice::Cancel};

public:
    explicit CConfirmUnlinkDialog(QWidget *pParent = nullptr)
        : QDialog(pParent)
    {
        mpLabel = new QLabel(tr("Which links should be removed from the selected instances?"));
        mpAllButton = new QPushButton(tr("All"));
        mpIncomingButton = new QPushButton(tr("Incoming Links"));
        mpOutgoingButton = new QPushButton(tr("Outgoing Links"));
        mpCancelButton = new QPushButton(tr("Cancel"));

        mpButtonLayout = new QHBoxLayout();
        mpButtonLayout->addWidget(mpAllButton);
        mpButtonLayout->addWidget(mpIncomingButton);
        mpButtonLayout->addWidget(mpOutgoingButton);
        mpButtonLayout->addWidget(mpCancelButton);

        mpDialogLayout = new QVBoxLayout();
        mpDialogLayout->addWidget(mpLabel);
        mpDialogLayout->addLayout(mpButtonLayout);
        setLayout(mpDialogLayout);

        connect(mpAllButton, &QPushButton::clicked, this, &CConfirmUnlinkDialog::OnAllClicked);
        connect(mpIncomingButton, &QPushButton::clicked, this, &CConfirmUnlinkDialog::OnIncomingClicked);
        connect(mpOutgoingButton, &QPushButton::clicked, this, &CConfirmUnlinkDialog::OnOutgoingClicked);
        connect(mpCancelButton, &QPushButton::clicked, this, &CConfirmUnlinkDialog::OnCancelClicked);
    }

    EChoice UserChoice() const { return mChoice; }

protected slots:
    void OnAllClicked()
    {
        mChoice = EChoice::All;
        accept();
    }

    void OnIncomingClicked()
    {
        mChoice = EChoice::IncomingOnly;
        accept();
    }

    void OnOutgoingClicked()
    {
        mChoice = EChoice::OutgoingOnly;
        accept();
    }

    void OnCancelClicked()
    {
        mChoice = EChoice::Cancel;
        reject();
    }
};

#endif // CCONFIRMUNLINKDIALOG_H
