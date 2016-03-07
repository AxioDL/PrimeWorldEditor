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
    enum EChoice
    {
        eAll,
        eIncomingOnly,
        eOutgoingOnly,
        eCancel
    };

private:
    QLabel *mpLabel;
    QPushButton *mpAllButton;
    QPushButton *mpIncomingButton;
    QPushButton *mpOutgoingButton;
    QPushButton *mpCancelButton;
    QHBoxLayout *mpButtonLayout;
    QVBoxLayout *mpDialogLayout;

    EChoice mChoice;

public:
    explicit CConfirmUnlinkDialog(QWidget *pParent = 0)
        : QDialog(pParent)
    {
        mpLabel = new QLabel("Which links should be removed from the selected instances?");
        mpAllButton = new QPushButton("All");
        mpIncomingButton = new QPushButton("Incoming Links");
        mpOutgoingButton = new QPushButton("Outgoing Links");
        mpCancelButton = new QPushButton("Cancel");

        mpButtonLayout = new QHBoxLayout();
        mpButtonLayout->addWidget(mpAllButton);
        mpButtonLayout->addWidget(mpIncomingButton);
        mpButtonLayout->addWidget(mpOutgoingButton);
        mpButtonLayout->addWidget(mpCancelButton);

        mpDialogLayout = new QVBoxLayout();
        mpDialogLayout->addWidget(mpLabel);
        mpDialogLayout->addLayout(mpButtonLayout);
        setLayout(mpDialogLayout);

        connect(mpAllButton, SIGNAL(clicked()), this, SLOT(OnAllClicked()));
        connect(mpIncomingButton, SIGNAL(clicked()), this, SLOT(OnIncomingClicked()));
        connect(mpOutgoingButton, SIGNAL(clicked()), this, SLOT(OnOutgoingClicked()));
        connect(mpCancelButton, SIGNAL(clicked()), this, SLOT(OnCancelClicked()));
    }

    inline EChoice UserChoice() const { return mChoice; }

protected slots:
    void OnAllClicked()
    {
        mChoice = eAll;
        accept();
    }

    void OnIncomingClicked()
    {
        mChoice = eIncomingOnly;
        accept();
    }

    void OnOutgoingClicked()
    {
        mChoice = eOutgoingOnly;
        accept();
    }

    void OnCancelClicked()
    {
        mChoice = eCancel;
        reject();
    }
};

#endif // CCONFIRMUNLINKDIALOG_H
