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
    , mIsEditable(true)
{
    setContextMenuPolicy(Qt::CustomContextMenu);

    // Set up UI
    mpResNameButton = new QPushButton(this);
    mpResNameButton->setFlat(true);
    mpResNameButton->setStyleSheet("text-align:left; font-size:10pt; margin:0px; padding-left:2px");
    
    mpSetButton = new QPushButton(this);
    mpSetButton->setToolTip("Use selected asset in Resource Browser");
    mpSetButton->setIcon(QIcon(":/icons/ArrowL_16px.png"));
    mpSetButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mpSetButton->setFixedSize(16, 16);

    mpClearButton = new QPushButton(this);
    mpClearButton->setToolTip("Clear");
    mpClearButton->setIcon(QIcon(":/icons/X_16px.png"));
    mpClearButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mpClearButton->setFixedSize(16, 16);
    
    mpFrameLayout = new QHBoxLayout(this);
    mpFrameLayout->setSpacing(2);
    mpFrameLayout->setContentsMargins(0, 0, 0, 0);
    mpFrameLayout->addWidget(mpResNameButton);
    mpFrameLayout->addWidget(mpSetButton);
    mpFrameLayout->addWidget(mpClearButton);
    mpFrame = new QFrame(this);
    mpFrame->setBackgroundRole(QPalette::AlternateBase);
    mpFrame->setLayout(mpFrameLayout);
    SetFrameVisible(true);

    mpLayout = new QVBoxLayout(this);
    mpLayout->addWidget(mpFrame);
    mpLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(mpLayout);

    // UI Connections
    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(CreateContextMenu(QPoint)));
    connect(mpResNameButton, SIGNAL(clicked()), this, SLOT(Find()));
    connect(mpSetButton, SIGNAL(clicked()), this, SLOT(Set()));
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

void CResourceSelector::SetFrameVisible(bool Visible)
{
    mpFrame->setFrameStyle(Visible ? QFrame::StyledPanel : QFrame::NoFrame);
    mpFrame->setAutoFillBackground(Visible);
}

void CResourceSelector::SetEditable(bool Editable)
{
    mpSetButton->setVisible(Editable);
    mpClearButton->setVisible(Editable);
    mIsEditable = Editable;
}

void CResourceSelector::UpdateUI()
{
    bool HasResource = mpResEntry != nullptr;

    // Update main UI
    mpResNameButton->setText(HasResource ? TO_QSTRING(mpResEntry->Name()) + "." + TO_QSTRING(mpResEntry->CookedExtension().ToString()) : "");
    mpResNameButton->setToolTip(HasResource ? TO_QSTRING(mpResEntry->CookedAssetPath(true)) : "");
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
    gpEdApp->clipboard()->setText(mpResNameButton->text());
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
    if (mpResEntry)
    {
        CResourceBrowser *pBrowser = gpEdApp->ResourceBrowser();
        pBrowser->SelectResource(mpResEntry);
        pBrowser->show();
        pBrowser->raise();
    }
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
