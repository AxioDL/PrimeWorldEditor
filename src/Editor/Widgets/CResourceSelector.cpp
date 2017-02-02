#include "CResourceSelector.h"
#include "Editor/CEditorApplication.h"
#include "Editor/UICommon.h"
#include "Editor/ResourceBrowser/CResourceBrowser.h"
#include <Core/GameProject/CResourceStore.h>
#include <Core/Resource/CResource.h>
#include <QAction>
#include <QClipboard>
#include <QMenu>

CResourceSelector::CResourceSelector(QWidget *pParent /*= 0*/)
    : QWidget(pParent)
    , mpResEntry(nullptr)
{
    setContextMenuPolicy(Qt::CustomContextMenu);

    // Set up UI
    mpResNameLabel = new QLabel(this);
    
    mpSetButton = new QPushButton(this);
    mpSetButton->setToolTip("Use selected asset in Resource Browser");
    mpSetButton->setIcon(QIcon(":/icons/ArrowL_16px.png"));
    mpSetButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mpSetButton->setFixedSize(16, 16);

    mpFindButton = new QPushButton(this);
    mpFindButton->setToolTip("Find in Resource Browser");
    mpFindButton->setIcon(QIcon(":/icons/Search_16px.png"));
    mpFindButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mpFindButton->setFixedSize(16, 16);

    mpClearButton = new QPushButton(this);
    mpClearButton->setToolTip("Clear");
    mpClearButton->setIcon(QIcon(":/icons/X_16px.png"));
    mpClearButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mpClearButton->setFixedSize(16, 16);
    
    mpLayout = new QHBoxLayout(this);
    mpLayout->setSpacing(2);
    mpLayout->setContentsMargins(0, 0, 0, 0);
    mpLayout->addWidget(mpResNameLabel);
    mpLayout->addWidget(mpSetButton);
    mpLayout->addWidget(mpFindButton);
    mpLayout->addWidget(mpClearButton);
    setLayout(mpLayout);

    // UI Connections
    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(CreateContextMenu(QPoint)));
    connect(mpSetButton, SIGNAL(clicked()), this, SLOT(Set()));
    connect(mpFindButton, SIGNAL(clicked()), this, SLOT(Find()));
    connect(mpClearButton, SIGNAL(clicked()), this, SLOT(Clear()));

    // Set up context menu
    mpEditAssetAction = new QAction("Edit", this);
    mpCopyNameAction = new QAction("Copy name", this);
    mpCopyPathAction = new QAction("Copy path", this);

    // Context menu connections
    connect(mpEditAssetAction, SIGNAL(triggered()), this, SLOT(EditAsset()));
    connect(mpCopyNameAction, SIGNAL(triggered()), this, SLOT(CopyName()));
    connect(mpCopyPathAction, SIGNAL(triggered()), this, SLOT(CopyPath()));

    UpdateUI();
}

void CResourceSelector::UpdateUI()
{
    bool HasResource = mpResEntry != nullptr;

    // Update main UI
    mpResNameLabel->setText(HasResource ? TO_QSTRING(mpResEntry->Name()) + "." + TO_QSTRING(mpResEntry->CookedExtension().ToString()) : "");
    mpResNameLabel->setToolTip(HasResource ? TO_QSTRING(mpResEntry->CookedAssetPath(true)) : "");
    mpFindButton->setEnabled(HasResource);
    mpClearButton->setEnabled(HasResource);

    // Update context menu
    mpEditAssetAction->setEnabled(HasResource);
    mpCopyNameAction->setEnabled(HasResource);
    mpCopyPathAction->setEnabled(HasResource);
}

void CResourceSelector::SetAllowedExtensions(const QString& /*rkExtension*/)
{
    // todo
}

void CResourceSelector::SetAllowedExtensions(const TStringList& /*rkExtensions*/)
{
    // todo
}

void CResourceSelector::SetResource(const CAssetID& rkID)
{
    mpResEntry = gpResourceStore->FindEntry(rkID);
    OnResourceChanged();
}

void CResourceSelector::SetResource(CResourceEntry *pEntry)
{
    mpResEntry = pEntry;
    OnResourceChanged();
}

void CResourceSelector::SetResource(CResource *pRes)
{
    mpResEntry = (pRes ? pRes->Entry() : nullptr);
    OnResourceChanged();
}

void CResourceSelector::CreateContextMenu(const QPoint& rkPoint)
{
    QMenu Menu;
    Menu.addAction(mpEditAssetAction);
    Menu.addSeparator();
    Menu.addAction(mpCopyNameAction);
    Menu.addAction(mpCopyPathAction);
    Menu.exec(mapToGlobal(rkPoint));
}

void CResourceSelector::EditAsset()
{
    gpEdApp->EditResource(mpResEntry);
}

void CResourceSelector::CopyName()
{
    gpEdApp->clipboard()->setText(mpResNameLabel->text());
}

void CResourceSelector::CopyPath()
{
    QString Text = (mpResEntry ? TO_QSTRING(mpResEntry->CookedAssetPath(true)) : "");
    gpEdApp->clipboard()->setText(Text);
}

void CResourceSelector::Set()
{
    // todo - validate this resource is a valid type
    CResourceBrowser *pBrowser = gpEdApp->ResourceBrowser();

    if (pBrowser->isVisible() && pBrowser->SelectedEntry())
    {
        mpResEntry = gpEdApp->ResourceBrowser()->SelectedEntry();
        OnResourceChanged();
    }
}

void CResourceSelector::Find()
{
    CResourceBrowser *pBrowser = gpEdApp->ResourceBrowser();
    pBrowser->SelectResource(mpResEntry);
    pBrowser->show();
    pBrowser->raise();
}

void CResourceSelector::Clear()
{
    mpResEntry = nullptr;
    OnResourceChanged();
}

void CResourceSelector::OnResourceChanged()
{
    UpdateUI();
    emit ResourceChanged(mpResEntry);
}
