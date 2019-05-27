#include "CResourceSelector.h"
#include "CSelectResourcePanel.h"
#include "Editor/CEditorApplication.h"
#include "Editor/UICommon.h"
#include "Editor/ResourceBrowser/CResourceBrowser.h"
#include "Editor/ResourceBrowser/CResourceMimeData.h"

#include <Core/GameProject/CResourceStore.h>
#include <Core/Resource/CResource.h>

#include <QAction>
#include <QClipboard>
#include <QDrag>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QMenu>

CResourceSelector::CResourceSelector(QWidget *pParent /*= 0*/)
    : QWidget(pParent)
    , mpResEntry(nullptr)
    , mIsEditable(true)
    , mIsDragging(false)
{
    setAcceptDrops(true);
    setContextMenuPolicy(Qt::CustomContextMenu);

    // Set up UI
    mpResNameButton = new QPushButton(this);
    mpResNameButton->setFlat(true);
    mpResNameButton->setStyleSheet("text-align:left; font-size:10pt; margin:0px; padding-left:2px");
    
    mpSelectButton = new QPushButton(this);
    mpSelectButton->setToolTip("Select Resource");
    mpSelectButton->setIcon(QIcon(":/icons/ArrowD_16px.svg"));
    mpSelectButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mpSelectButton->setFixedSize(16, 16);

    mpClearButton = new QPushButton(this);
    mpClearButton->setToolTip("Clear");
    mpClearButton->setIcon(QIcon(":/icons/X_16px.svg"));
    mpClearButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mpClearButton->setFixedSize(16, 16);
    
    mpFrameLayout = new QHBoxLayout(this);
    mpFrameLayout->setSpacing(2);
    mpFrameLayout->setContentsMargins(0, 0, 0, 0);
    mpFrameLayout->addWidget(mpResNameButton);
    mpFrameLayout->addWidget(mpSelectButton);
    mpFrameLayout->addWidget(mpClearButton);
    mpFrame = new QFrame(this);
    mpFrame->setBackgroundRole(QPalette::AlternateBase);
    mpFrame->setLayout(mpFrameLayout);
    SetFrameVisible(true);

    mpLayout = new QVBoxLayout(this);
    mpLayout->addWidget(mpFrame);
    mpLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(mpLayout);

    // Set up event filter
    mpResNameButton->installEventFilter(this);

    // UI Connections
    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(CreateContextMenu(QPoint)));
    connect(mpResNameButton, SIGNAL(clicked()), this, SLOT(Find()));
    connect(mpSelectButton, SIGNAL(clicked()), this, SLOT(Select()));
    connect(mpClearButton, SIGNAL(clicked()), this, SLOT(Clear()));
    connect(gpEdApp->ResourceBrowser(), SIGNAL(ResourceMoved(CResourceEntry*,CVirtualDirectory*,TString)), this, SLOT(OnResourceMoved(CResourceEntry*)));

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
    mpSelectButton->setVisible(Editable);
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

void CResourceSelector::SetTypeFilter(const CResTypeFilter& rkFilter)
{
    mTypeFilter = rkFilter;
    ASSERT(!mpResEntry || mTypeFilter.Accepts(mpResEntry));
}

void CResourceSelector::SetTypeFilter(EGame Game, const TString& rkTypeList)
{
    mTypeFilter.FromString(Game, rkTypeList);
    ASSERT(!mpResEntry || mTypeFilter.Accepts(mpResEntry));
}

void CResourceSelector::SetResource(const CAssetID& rkID)
{
    CResourceEntry *pNewEntry = gpResourceStore->FindEntry(rkID);

    if (mpResEntry != pNewEntry)
    {
        mpResEntry = pNewEntry;
        OnResourceChanged();
    }
}

void CResourceSelector::SetResource(CResourceEntry *pEntry)
{
    if (mpResEntry != pEntry)
    {
        mpResEntry = pEntry;
        OnResourceChanged();
    }
}

void CResourceSelector::SetResource(CResource *pRes)
{
    CResourceEntry *pNewEntry = (pRes ? pRes->Entry() : nullptr);

    if (mpResEntry != pNewEntry)
    {
        mpResEntry = pNewEntry;
        OnResourceChanged();
    }
}

// ************ INTERFACE ************
bool CResourceSelector::eventFilter(QObject *pWatched, QEvent *pEvent)
{
    if (pWatched == mpResNameButton)
    {
        if (pEvent->type() == QEvent::MouseButtonPress)
        {
            QMouseEvent *pMouseEvent = static_cast<QMouseEvent*>(pEvent);
            mousePressEvent(pMouseEvent);
            return false;
        }

        else if (pEvent->type() == QEvent::MouseButtonDblClick)
        {
            QMouseEvent *pMouseEvent = static_cast<QMouseEvent*>(pEvent);

            if (pMouseEvent->button() == Qt::LeftButton)
            {
                if (mpResEntry)
                    gpEdApp->EditResource(mpResEntry);

                return true;
            }
        }
    }

    return false;
}

// ************ DRAG ************
void CResourceSelector::mousePressEvent(QMouseEvent *pEvent)
{
    if (mpResNameButton->rect().contains(pEvent->pos()) && pEvent->button() == Qt::LeftButton)
    {
        mDragStartPosition = pEvent->pos();
        mIsDragging = true;
    }
}

void CResourceSelector::mouseMoveEvent(QMouseEvent *pEvent)
{
    if (mIsDragging)
    {
        if ( (pEvent->pos() - mDragStartPosition).manhattanLength() >= gpEdApp->startDragDistance() )
        {
            QDrag *pDrag = new QDrag(this);
            CResourceMimeData *pMimeData = new CResourceMimeData(mpResEntry);
            pDrag->setMimeData(pMimeData);
            pDrag->exec(Qt::CopyAction);
        }
    }
}

void CResourceSelector::mouseReleaseEvent(QMouseEvent *pEvent)
{
    if (pEvent->button() == Qt::LeftButton)
    {
        mIsDragging = false;
    }
}

// ************ DROP *************
void CResourceSelector::dragEnterEvent(QDragEnterEvent *pEvent)
{
    // Check whether the mime data is a valid format
    if (mIsEditable && (pEvent->possibleActions() & Qt::CopyAction))
    {
        const CResourceMimeData *pkData = qobject_cast<const CResourceMimeData*>(pEvent->mimeData());

        if (pkData && pkData->Directories().isEmpty() && pkData->Resources().size() == 1)
        {
            CResourceEntry *pEntry = pkData->Resources().front();

            if (!pEntry || mTypeFilter.Accepts(pEntry))
                pEvent->acceptProposedAction();
        }
    }
}

void CResourceSelector::dropEvent(QDropEvent *pEvent)
{
    // Set the new resource
    const CResourceMimeData *pkMimeData = qobject_cast<const CResourceMimeData*>(pEvent->mimeData());
    CResourceEntry *pEntry = pkMimeData->Resources().front();
    SetResource(pEntry);
}

// ************ SLOTS ************
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

void CResourceSelector::Select()
{
    new CSelectResourcePanel(this);
}

void CResourceSelector::Find()
{
    if (mpResEntry)
    {
        CResourceBrowser *pBrowser = gpEdApp->ResourceBrowser();
        pBrowser->SelectResource(mpResEntry, true);
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

void CResourceSelector::OnResourceMoved(CResourceEntry *pEntry)
{
    if (pEntry == mpResEntry)
        UpdateUI();
}
