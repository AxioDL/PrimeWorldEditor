#include "CSelectResourcePanel.h"
#include "ui_CSelectResourcePanel.h"
#include "Editor/CEditorApplication.h"
#include <Common/Math/MathUtil.h>
#include <QDesktopWidget>
#include <QDebug>

CSelectResourcePanel::CSelectResourcePanel(CResourceSelector *pSelector)
    : QFrame(pSelector)
    , mpUI(std::make_unique<Ui::CSelectResourcePanel>())
    , mpSelector(pSelector)
    , mModel(pSelector)
{
    setWindowFlags( windowFlags() | Qt::FramelessWindowHint | Qt::Window );

    mpUI->setupUi(this);
    mProxyModel.setSourceModel(&mModel);
    mpUI->ResourceTableView->setModel(&mProxyModel);

    // Signals/slots
    connect(gpEdApp, &QApplication::focusChanged, this, &CSelectResourcePanel::FocusChanged);
    connect(mpUI->SearchBar, &CTimedLineEdit::StoppedTyping, this, &CSelectResourcePanel::SearchStringChanged);
    connect(mpUI->ResourceTableView, &QTableView::clicked, this, &CSelectResourcePanel::ResourceClicked);

    // Determine size
    QPoint SelectorPos = pSelector->parentWidget()->mapToGlobal( pSelector->pos() );
    QRect ScreenRect = gpEdApp->desktop()->availableGeometry();

    int MaxWidthLeft = SelectorPos.x();
    int MaxWidthRight = ScreenRect.width() - SelectorPos.x() - pSelector->width();
    int MaxWidth = Math::Max(MaxWidthLeft, MaxWidthRight);

    int MaxHeightTop = SelectorPos.y();
    int MaxHeightBottom = ScreenRect.height() - SelectorPos.y() - pSelector->height();
    int MaxHeight = Math::Max(MaxHeightTop, MaxHeightBottom);

    QSize PanelSize(Math::Min(width(), MaxWidth), Math::Min(height(), MaxHeight));

    // Determine position; place wherever we have the most amount of space
    QPoint PanelPos;

    if (MaxHeightTop > MaxHeightBottom)
        PanelPos.ry() = SelectorPos.y() - PanelSize.height();
    else
        PanelPos.ry() = SelectorPos.y() + pSelector->height();

    if (MaxWidthLeft > MaxWidthRight)
        PanelPos.rx() = SelectorPos.x() + (pSelector->width() - PanelSize.width());
    else
        PanelPos.rx() = SelectorPos.x();

    // Clamp position to screen boundaries
    PanelPos.rx() = Math::Clamp(0, ScreenRect.width() - PanelSize.width(), PanelPos.x());
    PanelPos.ry() = Math::Clamp(0, ScreenRect.height() - PanelSize.height(), PanelPos.y());

    // Create widget geometry
    QRect PanelRect(PanelPos, PanelSize);
    setGeometry(PanelRect);

    // Jump to the currently-selected resource
    QModelIndex Index = mModel.InitialIndex();
    QModelIndex ProxyIndex = mProxyModel.mapFromSource(Index);

    mpUI->ResourceTableView->scrollTo(ProxyIndex, QAbstractItemView::PositionAtCenter);
    mpUI->ResourceTableView->selectionModel()->setCurrentIndex(ProxyIndex, QItemSelectionModel::ClearAndSelect);

    // Show
    show();
    mpUI->SearchBar->setFocus();
}

CSelectResourcePanel::~CSelectResourcePanel() = default;

// Slots
void CSelectResourcePanel::FocusChanged(QWidget*, QWidget *pNew)
{
    // Destroy when the panel loses focus
    if (pNew != this && !isAncestorOf(pNew))
        deleteLater();
}

void CSelectResourcePanel::SearchStringChanged(QString SearchString)
{
    mProxyModel.SetSearchString(SearchString);
}

void CSelectResourcePanel::ResourceClicked(QModelIndex Index)
{
    QModelIndex SourceIndex = mProxyModel.mapToSource(Index);
    CResourceEntry *pEntry = mModel.EntryForIndex(SourceIndex);
    mpSelector->SetResource(pEntry);
    close();
}
