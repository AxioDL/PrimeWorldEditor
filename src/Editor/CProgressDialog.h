#ifndef CPROGRESSDIALOG_H
#define CPROGRESSDIALOG_H

#include "IProgressNotifierUI.h"
#include "UICommon.h"
#include <Core/GameProject/CGameProject.h>
#include <Core/IProgressNotifier.h>

#include <QDialog>
#include <QFuture>
#include <QFutureWatcher>

#if WIN32
#include <QtWinExtras/QWinTaskbarButton>
#include <QtWinExtras/QWinTaskbarProgress>
#endif

namespace Ui {
class CProgressDialog;
}

class CProgressDialog : public IProgressNotifierUI
{
    Q_OBJECT
    Ui::CProgressDialog *mpUI;
    bool mUseBusyIndicator;
    bool mAlertOnFinish;
    bool mFinished;
    bool mCanceled;

#if WIN32
    QWinTaskbarProgress *mpTaskbarProgress;
#endif

public:
    explicit CProgressDialog(QString OperationName, bool UseBusyIndicator, bool AlertOnFinish, QWidget *pParent = 0);
    ~CProgressDialog();

    void DisallowCanceling();

    // IProgressNotifier interface
    virtual bool ShouldCancel() const;

    // Slots
public slots:
    void closeEvent(QCloseEvent *pEvent);
    void FinishAndClose();
    void CancelButtonClicked();
    void UpdateUI(const QString& rkTaskDesc, const QString& rkStepDesc, float ProgressPercent);

    // Results
protected:
    template<typename RetType>
    void InternalWaitForResults(QFuture<RetType> Future)
    {
        gpEdApp->SetEditorTicksEnabled(false);

        QFutureWatcher<RetType> Watcher;
        connect(&Watcher, SIGNAL(finished()), this, SLOT(FinishAndClose()));
        Watcher.setFuture(Future);
        exec();

        gpEdApp->SetEditorTicksEnabled(true);

        if (mAlertOnFinish)
            gpEdApp->alert(parentWidget());
    }

public:
    template<typename RetType>
    RetType WaitForResults(QFuture<RetType> Future)
    {
        InternalWaitForResults(Future);
        return Future.result();
    }

    template<>
    void WaitForResults(QFuture<void> Future)
    {
        InternalWaitForResults(Future);
    }
};

#endif // CPROGRESSDIALOG_H
