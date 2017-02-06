#include "WResourceSelector.h"
#include "WTexturePreviewPanel.h"
#include "Editor/UICommon.h"
#include <Core/GameProject/CResourceStore.h>

#include <QApplication>
#include <QCompleter>
#include <QDesktopWidget>
#include <QDirModel>
#include <QEvent>
#include <QFileDialog>

WResourceSelector::WResourceSelector(QWidget *parent)
    : QWidget(parent)
    // Preview Panel Members
    , mpPreviewPanel(nullptr)
    , mEnablePreviewPanel(true)
    , mPreviewPanelValid(false)
    , mShowingPreviewPanel(false)
    , mAdjustPreviewToParent(false)
    // Resource Members
    , mpResource(nullptr)
    , mResourceValid(false)
{
    // Create Widgets
    mUI.LineEdit = new QLineEdit(this);
    mUI.BrowseButton = new QPushButton(this);

    // Create Layout
    mUI.Layout = new QHBoxLayout(this);
    setLayout(mUI.Layout);
    mUI.Layout->addWidget(mUI.LineEdit);
    mUI.Layout->addWidget(mUI.BrowseButton);
    mUI.Layout->setContentsMargins(0,0,0,0);
    mUI.Layout->setSpacing(1);

    // Set Up Widgets
    mUI.LineEdit->installEventFilter(this);
    mUI.LineEdit->setMouseTracking(true);
    mUI.LineEdit->setMaximumHeight(23);
    mUI.LineEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    mUI.BrowseButton->installEventFilter(this);
    mUI.BrowseButton->setMouseTracking(true);
    mUI.BrowseButton->setText("...");
    mUI.BrowseButton->setMaximumSize(25, 23);
    mUI.BrowseButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    connect(mUI.LineEdit, SIGNAL(editingFinished()), this, SLOT(OnLineEditTextEdited()));
    connect(mUI.BrowseButton, SIGNAL(clicked()), this, SLOT(OnBrowseButtonClicked()));
}

WResourceSelector::~WResourceSelector()
{
    delete mpPreviewPanel;
}

bool WResourceSelector::event(QEvent *pEvent)
{
    if ((pEvent->type() == QEvent::Leave) || (pEvent->type() == QEvent::WindowDeactivate))
        HidePreviewPanel();

    return false;
}

bool WResourceSelector::eventFilter(QObject* /*pObj*/, QEvent *pEvent)
{
    if (pEvent->type() == QEvent::MouseMove)
        if (mEnablePreviewPanel)
            ShowPreviewPanel();

    return false;
}

bool WResourceSelector::IsSupportedExtension(const QString& rkExtension)
{
    foreach(const QString& rkStr, mSupportedExtensions)
        if (rkStr == rkExtension) return true;

    return false;
}

bool WResourceSelector::HasSupportedExtension(CResourceEntry *pEntry)
{
    return IsSupportedExtension(TO_QSTRING(pEntry->CookedExtension().ToString()));
}

void WResourceSelector::UpdateFrameColor()
{
    // Red frame should only display if the current path is either invalid or points to an entry of an invalid type.
    bool RedFrame = (!GetText().isEmpty() && !mpResource) || (mpResource && !mResourceValid);
    mUI.LineEdit->setStyleSheet(RedFrame ? "border: 1px solid red" : "");
    mUI.LineEdit->setFont(font());
}

// ************ GETTERS ************
CResourceEntry* WResourceSelector::GetResourceEntry()
{
    return mpResource;
}

CResource* WResourceSelector::GetResource()
{
    return mpResource->Load();
}

QString WResourceSelector::GetText()
{
    return mUI.LineEdit->text();
}

bool WResourceSelector::IsPreviewPanelEnabled()
{
    return mEnablePreviewPanel;
}

// ************ SETTERS ************
void WResourceSelector::SetResource(CResource *pRes)
{
    SetResource(pRes ? pRes->Entry() : nullptr);
}

void WResourceSelector::SetResource(CResourceEntry *pRes)
{
    if (mpResource != pRes)
    {
        mpResource = pRes;

        // We might prefer to have the line edit be cleared if pRes is null. However atm this function triggers
        // when the user types in a resource path so I'd prefer for the text not to be cleared out in that case
        if (mpResource)
        {
            TWideString Path = mpResource->CookedAssetPath(true);
            mUI.LineEdit->setText(TO_QSTRING(Path));
            mResourceValid = HasSupportedExtension(mpResource);
        }
        else
            mResourceValid = false;

        UpdateFrameColor();
        CreatePreviewPanel();
        emit ResourceChanged(mpResource);
    }
}

void WResourceSelector::SetResource(const CAssetID& rkID)
{
    CResourceEntry *pEntry = gpResourceStore->FindEntry(rkID);
    SetResource(pEntry);
}

void WResourceSelector::SetResource(const QString& rkRes)
{
    CResourceEntry *pEntry = gpResourceStore->FindEntry(TO_TWIDESTRING(rkRes));
    SetResource(pEntry);
}

void WResourceSelector::SetAllowedExtensions(const QString& rkExtension)
{
    TStringList list = TString(rkExtension.toStdString()).Split(",");
    SetAllowedExtensions(list);
}

void WResourceSelector::SetAllowedExtensions(const TStringList& rkExtensions)
{
    mSupportedExtensions.clear();
    for (auto it = rkExtensions.begin(); it != rkExtensions.end(); it++)
        mSupportedExtensions << TO_QSTRING(*it);
}

void WResourceSelector::SetText(const QString& rkResPath)
{
    mUI.LineEdit->setText(rkResPath);
    CResourceEntry *pEntry = gpResourceStore->FindEntry(TO_TWIDESTRING(rkResPath));
    SetResource(pEntry);
}

void WResourceSelector::SetPreviewPanelEnabled(bool Enabled)
{
    mEnablePreviewPanel = Enabled;
    if (!mPreviewPanelValid) CreatePreviewPanel();
}

void WResourceSelector::AdjustPreviewToParent(bool Adjust)
{
    mAdjustPreviewToParent = Adjust;
}

// ************ SLOTS ************
void WResourceSelector::OnLineEditTextEdited()
{
    SetResource(mUI.LineEdit->text());
}

void WResourceSelector::OnBrowseButtonClicked()
{
    // Construct filter string
    QString Filter;

    if (mSupportedExtensions.size() > 1)
    {
        QString All = "All allowed extensions (";

        for (int iExt = 0; iExt < mSupportedExtensions.size(); iExt++)
        {
            if (iExt > 0) All += " ";
            All += "*." + mSupportedExtensions[iExt];
        }
        All += ")";
        Filter += All + ";;";
    }

    for (int iExt = 0; iExt < mSupportedExtensions.size(); iExt++)
    {
        if (iExt > 0) Filter += ";;";
        Filter += UICommon::ExtensionFilterString(mSupportedExtensions[iExt]);
    }

    QString NewRes = UICommon::OpenFileDialog(this, "Select resource", Filter);

    if (!NewRes.isEmpty())
    {
        mUI.LineEdit->setText(NewRes);
        SetResource(NewRes);
    }
}

// ************ PRIVATE ************
void WResourceSelector::CreatePreviewPanel()
{
    delete mpPreviewPanel;
    mpPreviewPanel = nullptr;

    if (mResourceValid)
        mpPreviewPanel = IPreviewPanel::CreatePanel(mpResource->ResourceType(), this);

    if (!mpPreviewPanel) mPreviewPanelValid = false;

    else
    {
        mPreviewPanelValid = true;
        mpPreviewPanel->setWindowFlags(Qt::ToolTip);
        if (mResourceValid) mpPreviewPanel->SetResource(mpResource->Load());
    }
}

void WResourceSelector::ShowPreviewPanel()
{
    if (mPreviewPanelValid)
    {
        // Preferred panel point is lower-right, but can move if there's not enough room
        QPoint Position = parentWidget()->mapToGlobal(pos());
        QRect ScreenResolution = QApplication::desktop()->screenGeometry();
        QSize PanelSize = mpPreviewPanel->size();
        QPoint PanelPoint = Position;

        // Calculate parent adjustment with 9 pixels of buffer
        int ParentAdjustLeft = (mAdjustPreviewToParent ? pos().x() + 9 : 0);
        int ParentAdjustRight = (mAdjustPreviewToParent ? parentWidget()->width() - pos().x() + 9 : 0);

        // Is there enough space on the right?
        if (Position.x() + width() + PanelSize.width() + ParentAdjustRight >= ScreenResolution.width())
            PanelPoint.rx() -= PanelSize.width() + ParentAdjustLeft;
        else
            PanelPoint.rx() += width() + ParentAdjustRight;

        // Is there enough space on the bottom?
        if (Position.y() + PanelSize.height() >= ScreenResolution.height() - 30)
        {
            int Difference = Position.y() + PanelSize.height() - ScreenResolution.height() + 30;
            PanelPoint.ry() -= Difference;
        }

        mpPreviewPanel->move(PanelPoint);
        mpPreviewPanel->show();
        mShowingPreviewPanel = true;
    }
}

void WResourceSelector::HidePreviewPanel()
{
    if (mPreviewPanelValid && mShowingPreviewPanel)
    {
        mpPreviewPanel->hide();
        mShowingPreviewPanel = false;
    }
}
