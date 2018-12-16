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

    EChoice mChoice;

public:
    explicit CConfirmUnlinkDialog(QWidget *pParent = 0)
        : QDialog(pParent)
        , mChoice(EChoice::Cancel)
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
