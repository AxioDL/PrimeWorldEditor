#ifndef CUIRELAY_H
#define CUIRELAY_H

#include <Core/IUIRelay.h>
#include "CEditorApplication.h"
#include "WorldEditor/CWorldEditor.h"
#include "UICommon.h"

#include <QThread>

class CUIRelay : public QObject, public IUIRelay
{
    Q_OBJECT

    Qt::ConnectionType GetConnectionType()
    {
        bool IsUIThread = (QThread::currentThread() == gpEdApp->thread());
        return IsUIThread ? Qt::DirectConnection : Qt::BlockingQueuedConnection;
    }

public:
    explicit CUIRelay(QObject *pParent = 0)
        : QObject(pParent)
    {}

    // Note: All function calls should be deferred with QMetaObject::invokeMethod to ensure
    // that they run on the UI thread instead of whatever thread we happen to be on.
    virtual void ShowMessageBox(const TString& rkInfoBoxTitle, const TString& rkMessage)
    {
        QMetaObject::invokeMethod(this, "MessageBoxSlot", GetConnectionType(),
                                  Q_ARG(QString, TO_QSTRING(rkInfoBoxTitle)),
                                  Q_ARG(QString, TO_QSTRING(rkMessage)) );
    }

    virtual void ShowMessageBoxAsync(const TString& rkInfoBoxTitle, const TString& rkMessage)
    {
        QMetaObject::invokeMethod(this, "MessageBoxSlot", Qt::QueuedConnection,
                                  Q_ARG(QString, TO_QSTRING(rkInfoBoxTitle)),
                                  Q_ARG(QString, TO_QSTRING(rkMessage)) );
    }

    virtual bool AskYesNoQuestion(const TString& rkInfoBoxTitle, const TString& rkQuestion)
    {
        bool RetVal;
        QMetaObject::invokeMethod(this, "AskYesNoQuestionSlot", GetConnectionType(),
                                  Q_RETURN_ARG(bool, RetVal),
                                  Q_ARG(QString, TO_QSTRING(rkInfoBoxTitle)),
                                  Q_ARG(QString, TO_QSTRING(rkQuestion)) );
        return RetVal;
    }

    virtual bool OpenProject(const TString& kPath = "")
    {
        bool RetVal;
        QMetaObject::invokeMethod(this, "OpenProjectSlot", GetConnectionType(),
                                  Q_RETURN_ARG(bool, RetVal),
                                  Q_ARG(QString, TO_QSTRING(kPath)) );
        return RetVal;
    }

private slots:
    void MessageBoxSlot(const QString& rkInfoBoxTitle, const QString& rkMessage)
    {
        UICommon::InfoMsg(gpEdApp->WorldEditor(), rkInfoBoxTitle, rkMessage);
    }

    bool AskYesNoQuestionSlot(const QString& rkInfoBoxTitle, const QString& rkQuestion)
    {
        return UICommon::YesNoQuestion(gpEdApp->WorldEditor(), rkInfoBoxTitle, rkQuestion);
    }

    bool OpenProjectSlot(const QString& kPath)
    {
        return !kPath.isEmpty() ? gpEdApp->OpenProject(kPath) : UICommon::OpenProject();
    }
};

#endif // CUIRELAY_H
