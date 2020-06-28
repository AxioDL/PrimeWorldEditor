#include "CErrorLogDialog.h"
#include "ui_CErrorLogDialog.h"
#include "UICommon.h"
#include <Common/Log.h>

CErrorLogDialog::CErrorLogDialog(QWidget *pParent)
    : QDialog(pParent)
    , ui(std::make_unique<Ui::CErrorLogDialog>())
{
    ui->setupUi(this);
    connect(ui->CloseButton, &QPushButton::clicked, this, &CErrorLogDialog::close);
}

CErrorLogDialog::~CErrorLogDialog() = default;

bool CErrorLogDialog::GatherErrors()
{
    const TStringList& rkErrors = NLog::GetErrorLog();
    if (rkErrors.empty())
        return false;

    QString DialogString;

    for (const auto& rkError : rkErrors)
    {
        QString Error = TO_QSTRING(rkError);
        QString LineColor;

        if (Error.startsWith(QStringLiteral("ERROR: ")))
            LineColor = QStringLiteral("#ff0000");
        else if (Error.startsWith(QStringLiteral("Warning: ")))
            LineColor = QStringLiteral("#ff8000");

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
