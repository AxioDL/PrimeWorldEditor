#ifndef CLINKDIALOG_H
#define CLINKDIALOG_H

#include "Editor/WorldEditor/CStateMessageModel.h"
#include <QDialog>

namespace Ui {
class CLinkDialog;
}

class CWorldEditor;
struct SRayIntersection;

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

    void NewLink(CScriptObject *pSender, CScriptObject *pReceiver);
    void EditLink(CLink *pLink);

    void SetGame(CGameTemplate *pGame);
    void SetSender(CScriptObject *pSender);
    void SetReceiver(CScriptObject *pReceiver);
    uint32_t State() const;
    uint32_t Message() const;

    void UpdateOkEnabled();
    void UpdateSenderNameLabel();
    void UpdateReceiverNameLabel();

    CScriptObject* Sender() const { return mpSender; }
    CScriptObject* Receiver() const { return mpReceiver; }
    bool IsPicking() const { return mIsPicking; }
    bool IsPickingSender() const;
    bool IsPickingReceiver() const;

    void accept() override;

protected:
    void resizeEvent(QResizeEvent*) override;
    void showEvent(QShowEvent*) override;
    void closeEvent(QCloseEvent*) override;

private slots:
    void OnSwapClicked();
    void OnPickFromViewportClicked();
    void OnPickModeClick(const SRayIntersection& rkHit, QMouseEvent *pEvent);
    void OnPickModeExit();
    void OnPickFromListClicked();
};

#endif // CLINKDIALOG_H
