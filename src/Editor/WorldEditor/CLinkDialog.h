#ifndef CLINKDIALOG_H
#define CLINKDIALOG_H

#include "CStateMessageModel.h"
#include "ui_CLinkDialog.h"
#include <QDialog>

class CWorldEditor;

class CLinkDialog : public QDialog
{
    Q_OBJECT

    CWorldEditor *mpEditor;
    CGameTemplate *mpGame = nullptr;
    CScriptObject *mpSender = nullptr;
    CScriptObject *mpReceiver = nullptr;
    CLink *mpEditLink = nullptr;

    CStateMessageModel mSenderStateModel{CStateMessageModel::EType::States, this};
    CStateMessageModel mReceiverMessageModel{CStateMessageModel::EType::Messages, this};

    bool mIsPicking = false;

    std::unique_ptr<Ui::CLinkDialog> ui;

public:
    explicit CLinkDialog(CWorldEditor *pEditor, QWidget *parent = nullptr);
    ~CLinkDialog() override;

    void resizeEvent(QResizeEvent*) override;
    void showEvent(QShowEvent*) override;
    void closeEvent(QCloseEvent*) override;

    void NewLink(CScriptObject *pSender, CScriptObject *pReceiver);
    void EditLink(CLink *pLink);

    void SetGame(CGameTemplate *pGame);
    void SetSender(CScriptObject *pSender);
    void SetReceiver(CScriptObject *pReceiver);
    uint32 State() const;
    uint32 Message() const;

    void UpdateOkEnabled();
    void UpdateSenderNameLabel();
    void UpdateReceiverNameLabel();

    CScriptObject* Sender() const { return mpSender; }
    CScriptObject* Receiver() const { return mpReceiver; }
    bool IsPicking() const { return mIsPicking; }
    bool IsPickingSender() const { return mIsPicking && ui->SenderPickFromViewport->isChecked(); }
    bool IsPickingReceiver() const { return mIsPicking && ui->ReceiverPickFromViewport->isChecked(); }

public slots:
    void accept() override;
    void OnSwapClicked();
    void OnPickFromViewportClicked();
    void OnPickModeClick(const SRayIntersection& rkHit, QMouseEvent *pEvent);
    void OnPickModeExit();
    void OnPickFromListClicked();
};

#endif // CLINKDIALOG_H
