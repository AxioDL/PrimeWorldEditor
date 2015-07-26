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

    QFontMetrics metrics(mpTextLabel->font());
    this->resize(400, 100);
    this->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);
}

WStringPreviewPanel::~WStringPreviewPanel()
{
}

EResType WStringPreviewPanel::ResType()
{
    return eStringTable;
}

void WStringPreviewPanel::SetResource(CResource *pRes)
{
    mpTextLabel->clear();

    if ((pRes) && (pRes->Type() == eStringTable))
    {
        CStringTable *pString = static_cast<CStringTable*>(pRes);
        QString text;

        // Build text string using first four strings from table (or less if there aren't 4)
        u32 numStrings = (pString->GetStringCount() < 4 ? pString->GetStringCount() : 4);
        for (u32 iStr = 0; iStr < numStrings; iStr++)
        {
            text += QString::fromStdWString(pString->GetString(0, iStr));
            text += "\n";
        }

        // Build text layout to determine where to elide the label
        QTextLayout layout(text);


        mpTextLabel->setText(text);
    }
}
