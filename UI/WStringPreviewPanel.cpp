#include "WStringPreviewPanel.h"
#include <QFontMetrics>
#include <QTextLayout>
#include <Resource/CStringTable.h>

WStringPreviewPanel::WStringPreviewPanel(QWidget *pParent) : IPreviewPanel(pParent)
{
    mpTextLabel = new QLabel(this);
    mpTextLabel->setWordWrap(true);
    mpLayout = new QVBoxLayout(this);
    mpLayout->setAlignment(Qt::AlignTop);
    mpLayout->addWidget(mpTextLabel);
    setLayout(mpLayout);
}

WStringPreviewPanel::~WStringPreviewPanel()
{
}

QSize WStringPreviewPanel::sizeHint() const
{
    return QSize(400, 0);
}

EResType WStringPreviewPanel::ResType()
{
    return eStringTable;
}

void WStringPreviewPanel::SetResource(CResource *pRes)
{
    mpTextLabel->clear();

    if (pRes && (pRes->Type() == eStringTable))
    {
        CStringTable *pString = static_cast<CStringTable*>(pRes);
        QString text;

        for (u32 iStr = 0; iStr < pString->GetStringCount(); iStr++)
        {
            if (iStr > 0) text += "\n";
            text += QString::fromStdWString(pString->GetString(0, iStr));
        }

        mpTextLabel->setText(text);
    }
}
