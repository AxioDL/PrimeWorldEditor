#include "WCreateTab.h"
#include "ui_WCreateTab.h"

WCreateTab::WCreateTab(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WCreateTab)
{
    ui->setupUi(this);
}

WCreateTab::~WCreateTab()
{
    delete ui;
}
