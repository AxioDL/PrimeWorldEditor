#include "CErrorLogDialog.h"
#include "ui_CErrorLogDialog.h"
#include "UICommon.h"
#include <Common/Log.h>

CErrorLogDialog::CErrorLogDialog(QWidget *pParent)
    : QDialog(pParent)
    , ui(new Ui::CErrorLogDialog)
{
    ui->setupUi(this);
    connect(ui->CloseButton, SIGNAL(clicked()), this, SLOT(close()));
}

CErrorLogDialog::~CErrorLogDialog()
{
    delete ui;
}

bool CErrorLogDialog::GatherErrors()
{
    const TStringList& rkErrors = NLog::GetErrorLog();
    if (rkErrors.empty()) return false;

    QString DialogString;

    for (auto it = rkErrors.begin(); it != rkErrors.end(); it++)
    {
        QString Error = TO_QSTRING(*it);
        QString LineColor;

        if (Error.startsWith(QStringLiteral("ERROR: ")))
            LineColor = "#ff0000";
        else if (Error.startsWith(QStringLiteral("Warning: ")))
            LineColor = "#ff8000";

        QString FullLine = Error;

        if (!LineColor.isEmpty())
        {
            FullLine.prepend(QStringLiteral("<font color=\"%1\">").arg(LineColor));
            FullLine.append(QStringLiteral("</font>"));
        }
        FullLine.append(QStringLiteral("<br />"));

        DialogString += FullLine;
    }

    ui->ErrorLogTextEdit->setText(DialogString);
    NLog::ClearErrorLog();
    return true;
}
