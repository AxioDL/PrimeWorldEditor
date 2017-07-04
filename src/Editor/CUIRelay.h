#ifndef CUIRELAY_H
#define CUIRELAY_H

#include <Core/IUIRelay.h>
#include "CEditorApplication.h"
#include "WorldEditor/CWorldEditor.h"
#include "UICommon.h"

class CUIRelay : public QObject, public IUIRelay
{
    Q_OBJECT

public:
    explicit CUIRelay(QObject *pParent = 0)
        : QObject(pParent)
    {}

    // Note: All function calls should be deferred with QMetaObject::invokeMethod to ensure
    // that they run on the UI thread instead of whatever thread we happen to be on.
    virtual bool AskYesNoQuestion(const TString& rkInfoBoxTitle, const TString& rkQuestion)
    {
        bool RetVal;
        QMetaObject::invokeMethod(this, "AskYesNoQuestionSlot", Qt::BlockingQueuedConnection,
                                  Q_RETURN_ARG(bool, RetVal),
                                  Q_ARG(QString, TO_QSTRING(rkInfoBoxTitle)),
                                  Q_ARG(QString, TO_QSTRING(rkQuestion)) );
        return RetVal;
    }

public slots:
    bool AskYesNoQuestionSlot(const QString& rkInfoBoxTitle, const QString& rkQuestion)
    {
        return UICommon::YesNoQuestion(gpEdApp->WorldEditor(), rkInfoBoxTitle, rkQuestion);
    }
};

#endif // CUIRELAY_H
