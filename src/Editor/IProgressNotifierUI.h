#ifndef IPROGRESSNOTIFIERUI_H
#define IPROGRESSNOTIFIERUI_H

#include "UICommon.h"
#include <Core/IProgressNotifier.h>
#include <QDialog>

// IProgressNotifier subclass for UI classes (dialogs, etc)
class IProgressNotifierUI : public QDialog, public IProgressNotifier
{
public:
    explicit IProgressNotifierUI(QWidget *pParent = nullptr)
        : QDialog(pParent)
    {}

public slots:
    virtual void UpdateUI(const QString& rkTaskDesc, const QString& rkStepDesc, float ProgressPercent) = 0;

private:
    void UpdateProgress(const TString& rkTaskDesc, const TString& rkStepDesc, float ProgressPercent) final
    {
        // Defer the function call to make sure UI updates are done on the main thread
        QMetaObject::invokeMethod(this, "UpdateUI", Qt::AutoConnection,
                                  Q_ARG(QString, TO_QSTRING(rkTaskDesc)),
                                  Q_ARG(QString, TO_QSTRING(rkStepDesc)),
                                  Q_ARG(float, ProgressPercent) );
    }
};

#endif // IPROGRESSNOTIFIERUI_H
