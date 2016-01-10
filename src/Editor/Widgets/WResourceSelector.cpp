#include "WResourceSelector.h"
#include "WTexturePreviewPanel.h"
#include "Editor/UICommon.h"
#include <Core/Resource/CResCache.h>

#include <QApplication>
#include <QCompleter>
#include <QDesktopWidget>
#include <QDirModel>
#include <QEvent>
#include <QFileDialog>

WResourceSelector::WResourceSelector(QWidget *parent) : QWidget(parent)
{
    // Initialize Selector Members
    mShowEditButton = false;
    mShowExportButton = false;

    // Initialize Preview Panel Members
    mpPreviewPanel = nullptr;
    mEnablePreviewPanel = true;
    mPreviewPanelValid = false;
    mShowingPreviewPanel = false;
    mAdjustPreviewToParent = false;

    // Initialize Resource Members
    mResourceValid = false;

    // Create Widgets
    mUI.LineEdit = new QLineEdit(this);
    mUI.BrowseButton = new QPushButton(this);
    mUI.EditButton = new QPushButton("Edit", this);
    mUI.ExportButton = new QPushButton("Export", this);

    // Create Layout
    mUI.Layout = new QHBoxLayout(this);
    setLayout(mUI.Layout);
    mUI.Layout->addWidget(mUI.LineEdit);
    mUI.Layout->addWidget(mUI.BrowseButton);
    mUI.Layout->addWidget(mUI.EditButton);
    mUI.Layout->addWidget(mUI.ExportButton);
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
    mUI.EditButton->installEventFilter(this);
    mUI.EditButton->setMouseTracking(true);
    mUI.EditButton->setMaximumSize(50, 23);
    mUI.EditButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    mUI.EditButton->hide();
    mUI.ExportButton->installEventFilter(this);
    mUI.ExportButton->setMouseTracking(true);
    mUI.ExportButton->setMaximumSize(50, 23);
    mUI.ExportButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    mUI.ExportButton->hide();

    QCompleter *pCompleter = new QCompleter(this);
    pCompleter->setModel(new QDirModel(pCompleter));
    mUI.LineEdit->setCompleter(pCompleter);

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

bool WResourceSelector::IsSupportedExtension(const QString& extension)
{
    foreach(const QString& str, mSupportedExtensions)
        if (str == extension) return true;

    return false;
}

bool WResourceSelector::HasSupportedExtension(const CResourceInfo& rkRes)
{
    return IsSupportedExtension(TO_QSTRING(rkRes.Type().ToString()));
}

// ************ GETTERS ************
QString WResourceSelector::GetText()
{
    return mUI.LineEdit->text();
}

bool WResourceSelector::IsEditButtonEnabled()
{
    return mShowEditButton;
}

bool WResourceSelector::IsExportButtonEnabled()
{
    return mShowExportButton;
}

bool WResourceSelector::IsPreviewPanelEnabled()
{
    return mEnablePreviewPanel;
}


// ************ SETTERS ************
void WResourceSelector::SetResource(CResource *pRes)
{
    if (pRes)
        SetResource(CResourceInfo(pRes->ResID(), CFourCC(pRes->Source().GetFileExtension())));
    else
        SetResource(CResourceInfo());
}

void WResourceSelector::SetResource(const CResourceInfo& rkRes)
{
    mResource = rkRes;

    if (mResource.IsValid())
    {
        mResourceValid = HasSupportedExtension(rkRes);
        mUI.LineEdit->setText(TO_QSTRING(mResource.ToString()));
    }

    else
    {
        mResourceValid = false;
        mUI.LineEdit->clear();
    }

    CreatePreviewPanel();
    SetButtonsBasedOnResType();
}

void WResourceSelector::SetAllowedExtensions(const QString& extension)
{
    TStringList list = TString(extension.toStdString()).Split(",");
    SetAllowedExtensions(list);
}

void WResourceSelector::SetAllowedExtensions(const TStringList& extensions)
{
    mSupportedExtensions.clear();
    for (auto it = extensions.begin(); it != extensions.end(); it++)
        mSupportedExtensions << TO_QSTRING(*it);
}

void WResourceSelector::SetText(const QString& ResPath)
{
    mUI.LineEdit->setText(ResPath);
    LoadResource(ResPath);
}

void WResourceSelector::SetEditButtonEnabled(bool Enabled)
{
    mShowEditButton = Enabled;
    if (Enabled) mUI.EditButton->show();
    else mUI.EditButton->hide();
}

void WResourceSelector::SetExportButtonEnabled(bool Enabled)
{
    mShowExportButton = Enabled;
    if (Enabled) mUI.ExportButton->show();
    else mUI.ExportButton->hide();
}

void WResourceSelector::SetPreviewPanelEnabled(bool Enabled)
{
    mEnablePreviewPanel = Enabled;
    if (!mPreviewPanelValid) CreatePreviewPanel();
}

void WResourceSelector::AdjustPreviewToParent(bool adjust)
{
    mAdjustPreviewToParent = adjust;
}

// ************ SLOTS ************
void WResourceSelector::OnLineEditTextEdited()
{
    LoadResource(mUI.LineEdit->text());
}

void WResourceSelector::OnBrowseButtonClicked()
{
    // Construct filter string
    QString filter;

    if (mSupportedExtensions.size() > 1)
    {
        QString all = "All allowed extensions (";

        for (int iExt = 0; iExt < mSupportedExtensions.size(); iExt++)
        {
            if (iExt > 0) all += " ";
            all += "*." + mSupportedExtensions[iExt];
        }
        all += ")";
        filter += all + ";;";
    }

    for (int iExt = 0; iExt < mSupportedExtensions.size(); iExt++)
    {
        if (iExt > 0) filter += ";;";
        filter += UICommon::ExtensionFilterString(mSupportedExtensions[iExt]);
    }

    QString NewRes = QFileDialog::getOpenFileName(this, "Select resource", "", filter);

    if (!NewRes.isEmpty())
    {
        mUI.LineEdit->setText(NewRes);
        LoadResource(NewRes);
    }
}

void WResourceSelector::OnEditButtonClicked()
{
    Edit();
}

void WResourceSelector::OnExportButtonClicked()
{
    Export();
}

// ************ PRIVATE ************
// Should the resource selector handle edit/export itself
// or delegate it entirely to the signals?
void WResourceSelector::Edit()
{
    emit EditResource(mResource);
}

void WResourceSelector::Export()
{
    emit ExportResource(mResource);
}

void WResourceSelector::LoadResource(const QString& ResPath)
{
    mResource = CResourceInfo();

    TString PathStr = ResPath.toStdString();
    TString Ext = PathStr.GetFileExtension();

    if (IsSupportedExtension(TO_QSTRING(Ext)))
    {
        mResource = CResourceInfo(TO_TSTRING(ResPath));
        mResourceValid = mResource.IsValid();

        if (mPreviewPanelValid) mpPreviewPanel->SetResource(mResource.Load());
    }
    else mResourceValid = false;

    SetButtonsBasedOnResType();
    CreatePreviewPanel();
    emit ResourceChanged(ResPath);
}

void WResourceSelector::CreatePreviewPanel()
{
    delete mpPreviewPanel;
    mpPreviewPanel = nullptr;

    if (mResourceValid)
        mpPreviewPanel = IPreviewPanel::CreatePanel(CResource::ResTypeForExtension(mResource.Type()), this);

    if (!mpPreviewPanel) mPreviewPanelValid = false;

    else
    {
        mPreviewPanelValid = true;
        mpPreviewPanel->setWindowFlags(Qt::ToolTip);
        if (mResourceValid) mpPreviewPanel->SetResource(mResource.Load());
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

void WResourceSelector::SetButtonsBasedOnResType()
{
    // Basically this function sets whether the "Export" and "Edit"
    // buttons are present based on the resource type.
    if (!mResource.IsValid())
    {
        SetEditButtonEnabled(false);
        SetExportButtonEnabled(false);
    }

    else switch (CResource::ResTypeForExtension(mResource.Type()))
    {
    // Export button should be enabled here because CTexture already has a DDS export function
    // However, need to figure out what sort of interface to create to do it. Disabling until then.
    case eTexture:
        SetEditButtonEnabled(false);
        SetExportButtonEnabled(false);
        break;
    default:
        SetEditButtonEnabled(false);
        SetExportButtonEnabled(false);
        break;
    }
}
