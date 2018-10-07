#include "WScanPreviewPanel.h"
#include "ui_WScanPreviewPanel.h"
#include "WStringPreviewPanel.h"
#include <Core/Resource/CScan.h>

WScanPreviewPanel::WScanPreviewPanel(QWidget *pParent)
    : IPreviewPanel(pParent)
    , ui(new Ui::WScanPreviewPanel)
{
    ui->setupUi(this);

    ui->ScanTextWidget->setFrameShape(QFrame::NoFrame);
    ui->ScanTextWidget->layout()->setContentsMargins(9,0,9,9);
}

WScanPreviewPanel::~WScanPreviewPanel()
{
    delete ui;
}

QSize WScanPreviewPanel::sizeHint() const
{
    return QSize(400, 0);
}

EResType WScanPreviewPanel::ResType()
{
    return eScan;
}

void WScanPreviewPanel::SetResource(CResource *pRes)
{
    // Clear existing UI
    ui->ScanTypeLabel->clear();
    ui->ScanSpeedLabel->clear();
    ui->ScanCategoryLabel->clear();

    // Set up new UI
    if (pRes && (pRes->Type() == eScan))
    {
        CScan *pScan = static_cast<CScan*>(pRes);

        // Scan type
        if (pScan->IsImportant())
            ui->ScanTypeLabel->setText("<b><font color=\"red\">Important</font></b>");
        else
        {
            if (pScan->Game() <= EGame::Prime)
                ui->ScanTypeLabel->setText("<b><font color=\"#FF9030\">Normal</font></b>");
            else
                ui->ScanTypeLabel->setText("<b><font color=\"#A0A0FF\">Normal</font></b>");
        }

        // Scan speed
        if (pScan->IsSlow())
            ui->ScanSpeedLabel->setText("<b><font color=\"blue\">Slow</font></b>");
        else
            ui->ScanSpeedLabel->setText("<b><font color=\"green\">Fast</font></b>");

        // Scan category
        switch (pScan->LogbookCategory())
        {
        case CScan::eNone:
            ui->ScanCategoryLabel->setText("<b>None</b>");
            break;
        case CScan::eChozoLore:
            ui->ScanCategoryLabel->setText("<b>Chozo Lore</b>");
            break;
        case CScan::ePirateData:
            ui->ScanCategoryLabel->setText("<b>Pirate Data</b>");
            break;
        case CScan::eCreatures:
            ui->ScanCategoryLabel->setText("<b>Creatures</b>");
            break;
        case CScan::eResearch:
            ui->ScanCategoryLabel->setText("<b>Research</b>");
            break;
        }

        // Scan text
        ui->ScanTextWidget->SetResource(pScan->ScanText());

        // Show logbook category? (Yes on MP1, no on MP2+)
        if (pScan->Game() <= EGame::Prime)
        {
            ui->CategoryInfoLabel->show();
            ui->ScanCategoryLabel->show();
        }

        else
        {
            ui->CategoryInfoLabel->hide();
            ui->ScanCategoryLabel->hide();
        }
    }

    else
        ui->ScanTextWidget->SetResource(nullptr);
}
