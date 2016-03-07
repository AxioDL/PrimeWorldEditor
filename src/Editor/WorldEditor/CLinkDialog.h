#ifndef CLINKDIALOG_H
#define CLINKDIALOG_H

#include "CStateMessageModel.h"
#include <QDialog>

namespace Ui {
class CLinkDialog;
}

class CWorldEditor;

class CLinkDialog : public QDialog
{
    Q_OBJECT

    CWorldEditor *mpEditor;
    CMasterTemplate *mpMaster;
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

    void SetMaster(CMasterTemplate *pMaster);
    void SetSender(CScriptObject *pSender);
    void SetReceiver(CScriptObject *pReceiver);
    u32 State() const;
    u32 Message() const;

    void UpdateOkEnabled();
    void UpdateSenderNameLabel();
    void UpdateReceiverNameLabel();

    inline CScriptObject* Sender() const { return mpSender; }
    inline CScriptObject* Receiver() const { return mpReceiver; }

public slots:
    void accept();
    void OnSwapClicked();
    void OnPickFromViewportClicked();
    void OnPickModeClick(const SRayIntersection& rkHit, QMouseEvent *pEvent);
    void OnPickModeExit();
    void OnPickFromListClicked();
};

#endif // CLINKDIALOG_H
