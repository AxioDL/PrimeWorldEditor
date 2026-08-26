#ifndef IPROGRESSNOTIFIERUI_H
#define IPROGRESSNOTIFIERUI_H

#include "Editor/UICommon.h"
#include <Core/IProgressNotifier.h>
#include <QDialog>

// IProgressNotifier subclass for UI classes (dialogs, etc)
class IProgressNotifierUI : public QDialog, public IProgressNotifier
{
    Q_OBJECT

public:
    explicit IProgressNotifierUI(QWidget *pParent = nullptr)
        : QDialog(pParent)
    {}

public slots:
    virtual void UpdateUI(const QString& rkTaskDesc, const QString& rkStepDesc, float ProgressPercent) = 0;

private:
    void UpdateProgress(const std::string& rkTaskDesc, const std::string& rkStepDesc, float ProgressPercent) final
    {
        // Defer the function call to make sure UI updates are done on the main thread
        QMetaObject::invokeMethod(this, &IProgressNotifierUI::UpdateUI, Qt::AutoConnection,
                                  TO_QSTRING(rkTaskDesc), TO_QSTRING(rkStepDesc), ProgressPercent);
    }
};

#endif // IPROGRESSNOTIFIERUI_H
