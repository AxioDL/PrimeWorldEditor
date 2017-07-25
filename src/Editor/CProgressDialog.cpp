#include "CProgressDialog.h"
#include "ui_CProgressDialog.h"
#include "CEditorApplication.h"
#include <QCloseEvent>

CProgressDialog::CProgressDialog(QString OperationName, bool UseBusyIndicator, bool AlertOnFinish, QWidget *pParent)
    : IProgressNotifierUI(pParent)
    , mpUI(new Ui::CProgressDialog)
    , mUseBusyIndicator(UseBusyIndicator)
    , mAlertOnFinish(AlertOnFinish)
    , mFinished(false)
    , mCanceled(false)
{
    mpUI->setupUi(this);
    mpUI->ProgressBar->setMinimum(0);
    mpUI->ProgressBar->setMaximum(UseBusyIndicator ? 0 : 10000);
    setWindowTitle(OperationName);

#if WIN32
    QWinTaskbarButton *pButton = new QWinTaskbarButton(this);
    QWindow *pWindow = UICommon::FindWidgetWindowHandle( parentWidget() );

    if (pWindow)
    {
        pButton->setWindow(pWindow);
        mpTaskbarProgress = pButton->progress();
        mpTaskbarProgress->setMinimum(0);
        mpTaskbarProgress->setMaximum(UseBusyIndicator ? 0 : 10000);
        mpTaskbarProgress->show();
    }
    else
        mpTaskbarProgress = nullptr;
    
    setWindowFlags(windowFlags() | Qt::MSWindowsFixedSizeDialogHint);
#endif

    connect(mpUI->CancelButton, SIGNAL(pressed()), this, SLOT(CancelButtonClicked()));
}

CProgressDialog::~CProgressDialog()
{
    delete mpUI;
}

void CProgressDialog::DisallowCanceling()
{
    mpUI->CancelButton->setHidden(true);
}

bool CProgressDialog::ShouldCancel() const
{
    return mCanceled;
}

void CProgressDialog::closeEvent(QCloseEvent *pEvent)
{
    if (!mFinished)
    {
        CancelButtonClicked();
        pEvent->ignore();
    }
    else
    {
        pEvent->accept();

#if WIN32
        if (mpTaskbarProgress)
        {
            mpTaskbarProgress->reset();
            mpTaskbarProgress->hide();
        }
#endif
    }
}

void CProgressDialog::FinishAndClose()
{
    mFinished = true;
    close();
}

void CProgressDialog::CancelButtonClicked()
{
    mCanceled = true;
    mpUI->CancelButton->setEnabled(false);
}

void CProgressDialog::UpdateUI(const QString& rkTaskDesc, const QString& rkStepDesc, float ProgressPercent)
{
    mpUI->TaskLabel->setText(rkTaskDesc);
    mpUI->StepLabel->setText(rkStepDesc);

    if (rkStepDesc.isEmpty() && !mpUI->StepLabel->isHidden())
    {
        mpUI->StepLabel->hide();
        mpUI->TaskInfoBoxLayout->removeWidget(mpUI->StepLabel);
        mpUI->TaskInfoBoxLayout->removeItem(mpUI->LabelSpacer);
    }
    else if (!rkStepDesc.isEmpty() && mpUI->StepLabel->isHidden())
    {
        mpUI->StepLabel->show();
        mpUI->TaskInfoBoxLayout->addWidget(mpUI->StepLabel);
        mpUI->TaskInfoBoxLayout->addItem(mpUI->LabelSpacer);
    }

    if (!mUseBusyIndicator)
    {
        int ProgressValue = 10000 * ProgressPercent;
        mpUI->ProgressBar->setValue(ProgressValue);

#if WIN32
        if (mpTaskbarProgress)
            mpTaskbarProgress->setValue(ProgressValue);
#endif
    }
}
