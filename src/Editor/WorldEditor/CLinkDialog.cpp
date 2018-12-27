#include "CLinkDialog.h"
#include "CSelectInstanceDialog.h"
#include "CStateMessageModel.h"
#include "CWorldEditor.h"
#include "Editor/Undo/CAddLinkCommand.h"
#include "Editor/Undo/CEditLinkCommand.h"
#include <Core/Resource/Script/CScriptObject.h>

CLinkDialog::CLinkDialog(CWorldEditor *pEditor, QWidget *pParent /*= 0*/)
    : QDialog(pParent)
    , ui(new Ui::CLinkDialog)
    , mpEditor(pEditor)
    , mpGame(nullptr)
    , mpSender(nullptr)
    , mpReceiver(nullptr)
    , mSenderStateModel(CStateMessageModel::EType::States, this)
    , mReceiverMessageModel(CStateMessageModel::EType::Messages, this)
    , mIsPicking(false)
    , mpEditLink(nullptr)
{
    ui->setupUi(this);
    ui->SenderStateComboBox->setModel(&mSenderStateModel);
    ui->ReceiverMessageComboBox->setModel(&mReceiverMessageModel);

    connect(ui->SwapButton, SIGNAL(clicked()), this, SLOT(OnSwapClicked()));
    connect(ui->SenderPickFromViewport, SIGNAL(clicked()), this, SLOT(OnPickFromViewportClicked()));
    connect(ui->SenderPickFromList, SIGNAL(clicked()), this, SLOT(OnPickFromListClicked()));
    connect(ui->ReceiverPickFromViewport, SIGNAL(clicked()), this, SLOT(OnPickFromViewportClicked()));
    connect(ui->ReceiverPickFromList, SIGNAL(clicked()), this, SLOT(OnPickFromListClicked()));
}

CLinkDialog::~CLinkDialog()
{
    delete ui;
}

void CLinkDialog::resizeEvent(QResizeEvent *)
{
    UpdateSenderNameLabel();
    UpdateReceiverNameLabel();
}

void CLinkDialog::showEvent(QShowEvent *)
{
    // This is needed to get the labels to elide correctly when the window is first shown. It shouldn't be
    // needed because showing the window generates a resize event, but for some reason it is, so whatever.
    UpdateSenderNameLabel();
    UpdateReceiverNameLabel();
}

void CLinkDialog::closeEvent(QCloseEvent *)
{
    if (mIsPicking)
        mpEditor->ExitPickMode();
}

void CLinkDialog::NewLink(CScriptObject *pSender, CScriptObject *pReceiver)
{
    mpEditLink = nullptr;
    SetSender(pSender);
    SetReceiver(pReceiver);
    if (pSender)   ui->SenderStateComboBox->setCurrentIndex(0);
    if (pReceiver) ui->ReceiverMessageComboBox->setCurrentIndex(0);
}

void CLinkDialog::EditLink(CLink *pLink)
{
    mpEditLink = pLink;
    CScriptObject *pSender = pLink->Sender();
    CScriptObject *pReceiver = pLink->Receiver();
    SetSender(pSender);
    SetReceiver(pReceiver);

    if (pSender)
        ui->SenderStateComboBox->setCurrentIndex(mSenderStateModel.StateIndex(pLink->State()));
    if (pReceiver)
        ui->ReceiverMessageComboBox->setCurrentIndex(mReceiverMessageModel.MessageIndex(pLink->Message()));
}

void CLinkDialog::SetGame(CGameTemplate *pGame)
{
    if (mpGame != pGame)
    {
        mpGame = pGame;
        mSenderStateModel.SetGameTemplate(pGame);
        mReceiverMessageModel.SetGameTemplate(pGame);
    }
}

void CLinkDialog::SetSender(CScriptObject *pSender)
{
    bool HadSender = mpSender != nullptr;
    mpSender = pSender;
    mSenderStateModel.SetScriptTemplate(pSender ? pSender->Template() : nullptr);
    UpdateSenderNameLabel();
    UpdateOkEnabled();

    if (pSender)
    {
        if (!HadSender) ui->SenderStateComboBox->setCurrentIndex(0);
        ui->SenderStateComboBox->setEnabled(true);
    }
    else
    {
        ui->SenderStateComboBox->setCurrentIndex(-1);
        ui->SenderStateComboBox->setEnabled(false);
    }
}

void CLinkDialog::SetReceiver(CScriptObject *pReceiver)
{
    bool HadReceiver = mpReceiver != nullptr;
    mpReceiver = pReceiver;
    mReceiverMessageModel.SetScriptTemplate(pReceiver ? pReceiver->Template() : nullptr);
    UpdateReceiverNameLabel();
    UpdateOkEnabled();

    if (pReceiver)
    {
        if (!HadReceiver) ui->ReceiverMessageComboBox->setCurrentIndex(0);
        ui->ReceiverMessageComboBox->setEnabled(true);
    }
    else
    {
        ui->ReceiverMessageComboBox->setCurrentIndex(-1);
        ui->ReceiverMessageComboBox->setEnabled(false);
    }
}

uint32 CLinkDialog::State() const
{
    return mSenderStateModel.State(ui->SenderStateComboBox->currentIndex());
}

uint32 CLinkDialog::Message() const
{
    return mReceiverMessageModel.Message(ui->ReceiverMessageComboBox->currentIndex());
}

void CLinkDialog::UpdateOkEnabled()
{
    ui->ButtonBox->button(QDialogButtonBox::Ok)->setEnabled( Sender() && Receiver() );
}

void CLinkDialog::UpdateSenderNameLabel()
{
    QString Text = (mpSender ? TO_QSTRING(mpSender->InstanceName()) : "<i>No sender</i>");
    ui->SenderNameLabel->setToolTip(Text);

    QFontMetrics Metrics(ui->SenderNameLabel->font());
    QString Elided = Metrics.elidedText(Text, Qt::ElideRight, ui->SenderNameLabel->width() - (ui->SenderNameLabel->frameWidth() * 2));
    ui->SenderNameLabel->setText(Elided);

    ui->SenderGroupBox->setTitle(mpSender ? "Sender - " + TO_QSTRING(mpSender->Template()->Name()) : "Sender");
}

void CLinkDialog::UpdateReceiverNameLabel()
{
    QString Text = (mpReceiver ? TO_QSTRING(mpReceiver->InstanceName()) : "<i>No receiver</i>");
    ui->ReceiverNameLabel->setToolTip(Text);

    QFontMetrics Metrics(ui->ReceiverNameLabel->font());
    QString Elided = Metrics.elidedText(Text, Qt::ElideRight, ui->ReceiverNameLabel->width() - (ui->ReceiverNameLabel->frameWidth() * 2));
    ui->ReceiverNameLabel->setText(Elided);

    ui->ReceiverGroupBox->setTitle(mpReceiver ? "Receiver - " + TO_QSTRING(mpReceiver->Template()->Name()) : "Receiver");
}

// ************ PUBLIC SLOTS ************
void CLinkDialog::accept()
{
    CLink Link(mpEditor->ActiveArea(), State(), Message(), Sender()->InstanceID(), Receiver()->InstanceID());

    if (!mpEditLink)
    {
        CAddLinkCommand *pCmd = new CAddLinkCommand(mpEditor, Link);
        mpEditor->UndoStack().push(pCmd);
    }

    else if (Link != *mpEditLink)
    {
        CEditLinkCommand *pCmd = new CEditLinkCommand(mpEditor, mpEditLink, Link);
        mpEditor->UndoStack().push(pCmd);
    }

    QDialog::accept();
}

void CLinkDialog::OnSwapClicked()
{
    CScriptObject *pSender = mpReceiver;
    CScriptObject *pReceiver = mpSender;
    SetSender(pSender);
    SetReceiver(pReceiver);
}

void CLinkDialog::OnPickFromViewportClicked()
{
    QPushButton *pButton = qobject_cast<QPushButton*>(sender());

    if (pButton && pButton->isChecked())
    {
        mpEditor->EnterPickMode(ENodeType::Script, true, false, false);
        connect(mpEditor, SIGNAL(PickModeClick(SRayIntersection,QMouseEvent*)), this, SLOT(OnPickModeClick(SRayIntersection,QMouseEvent*)));
        connect(mpEditor, SIGNAL(PickModeExited()), this, SLOT(OnPickModeExit()));

        QPushButton *pOtherButton = (pButton == ui->SenderPickFromViewport ? ui->ReceiverPickFromViewport : ui->SenderPickFromViewport);
        pOtherButton->setChecked(false);

        mIsPicking = true;
    }

    else
        mpEditor->ExitPickMode();
}

void CLinkDialog::OnPickModeClick(const SRayIntersection& rkHit, QMouseEvent* /*pEvent*/)
{
    CScriptNode *pScript = static_cast<CScriptNode*>(rkHit.pNode);

    if (ui->SenderPickFromViewport->isChecked())
        SetSender(pScript->Instance());
    else
        SetReceiver(pScript->Instance());

    mpEditor->ExitPickMode();
}

void CLinkDialog::OnPickModeExit()
{
    ui->SenderPickFromViewport->setChecked(false);
    ui->ReceiverPickFromViewport->setChecked(false);
    disconnect(mpEditor, SIGNAL(PickModeClick(SRayIntersection,QMouseEvent*)), this, 0);
    disconnect(mpEditor, SIGNAL(PickModeExited()), this, 0);
    mIsPicking = false;
}

void CLinkDialog::OnPickFromListClicked()
{
    CSelectInstanceDialog Dialog(mpEditor, this);
    Dialog.exec();
    CScriptObject *pResult = Dialog.SelectedInstance();

    if (pResult)
    {
        if (sender() == ui->SenderPickFromList)
            SetSender(pResult);
        else
            SetReceiver(pResult);
    }
}
