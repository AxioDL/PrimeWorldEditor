#include "WStringPreviewPanel.h"
#include "Editor/UICommon.h"
#include <Core/Resource/CStringTable.h>

#include <QFontMetrics>
#include <QTextLayout>

WStringPreviewPanel::WStringPreviewPanel(QWidget *pParent)
    : IPreviewPanel(pParent)
{
    mpLayout = new QVBoxLayout(this);
    mpLayout->setAlignment(Qt::AlignTop);
    mpLayout->setSpacing(0);
    setLayout(mpLayout);
}

WStringPreviewPanel::~WStringPreviewPanel()
{
}

QSize WStringPreviewPanel::sizeHint() const
{
    return QSize(400, 0);
}

EResourceType WStringPreviewPanel::ResType()
{
    return EResourceType::StringTable;
}

void WStringPreviewPanel::SetResource(CResource *pRes)
{
    foreach(const QLabel *pLabel, mLabels)
        delete pLabel;
    mLabels.clear();

    if (pRes && (pRes->Type() == EResourceType::StringTable))
    {
        CStringTable *pString = static_cast<CStringTable*>(pRes);
        mLabels.reserve(pString->NumStrings());

        for (uint32 iStr = 0; iStr < pString->NumStrings(); iStr++)
        {
            QString text = TO_QSTRING(pString->String("ENGL", iStr));
            QLabel *pLabel = new QLabel(text, this);
            pLabel->setWordWrap(true);
            pLabel->setFrameStyle(QFrame::Plain | QFrame::Box);
            pLabel->setMargin(3);
            mLabels.push_back(pLabel);
            mpLayout->addWidget(pLabel);
        }
    }
}
