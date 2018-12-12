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
    CGameTemplate *mpGame;
    CScriptObject *mpSender;
    CScriptObject *mpReceiver;
    CLink *mpEditLink;

    CStateMessageModel mSenderStateModel;
    CStateMessageModel mReceiverMessageModel;

    bool mIsPicking;

    Ui::CLinkDialog *ui;

public:
    explicit CLinkDialog(CWorldEditor *pEditor, QWidget *parent = 0);
    ~CLinkDialog();
    void resizeEvent(QResizeEvent *);
    void showEvent(QShowEvent *);
    void closeEvent(QCloseEvent *);

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

    inline CScriptObject* Sender() const { return mpSender; }
    inline CScriptObject* Receiver() const { return mpReceiver; }
    inline bool IsPicking() const { return mIsPicking; }
    inline bool IsPickingSender() const { return mIsPicking && ui->SenderPickFromViewport->isChecked(); }
    inline bool IsPickingReceiver() const { return mIsPicking && ui->ReceiverPickFromViewport->isChecked(); }

public slots:
    void accept();
    void OnSwapClicked();
    void OnPickFromViewportClicked();
    void OnPickModeClick(const SRayIntersection& rkHit, QMouseEvent *pEvent);
    void OnPickModeExit();
    void OnPickFromListClicked();
};

#endif // CLINKDIALOG_H
